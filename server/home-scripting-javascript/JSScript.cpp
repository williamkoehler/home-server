#include "JSScript.hpp"
#include "JSScriptSource.hpp"
#include <home-scripting/utils/Event.hpp>
#include <home-scripting/utils/Value.hpp>

#include "JSUtils.hpp"
#include "main/JSDevice.hpp"
#include "main/JSRoom.hpp"
#include "utils/JSValue.hpp"

#define ATTRIBUTES_PROPERTY ("attributes")
#define PROPERTIES_PROPERTY ("properties")
#define METHODS_PROPERTY ("methods")
#define EVENTS_PROPERTY ("events")

#define INITIALIZE_FUNCTION ("_initialize")

extern "C"
{
    // Interrupt function called by the duktape engine
    // This allows the interruption of any javascript code that runs too long
    duk_ret_t duk_exec_timeout(void* udata)
    {
        server::scripting::javascript::JSScript* script = (server::scripting::javascript::JSScript*)udata;
        return script->CheckTimeout();
    }
}

void duk_log_dump(duk_context* context)
{
    duk_push_context_dump(context);
    LOG_INFO("Dump: {}", std::string(duk_get_string(context, -1)));
    duk_pop(context);
}

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            const char* AttributesProperty = ATTRIBUTES_PROPERTY;
            const size_t AttributesPropertySize = std::size(ATTRIBUTES_PROPERTY) - 1;

            const char* PropertiesProperty = PROPERTIES_PROPERTY;
            const size_t PropertiesPropertySize = std::size(PROPERTIES_PROPERTY) - 1;

            const char* EventsProperty = EVENTS_PROPERTY;
            const size_t EventsPropertySize = std::size(EVENTS_PROPERTY) - 1;

            const char* InitializeFunction = INITIALIZE_FUNCTION;
            const size_t InitializeFunctionSize = std::size(INITIALIZE_FUNCTION) - 1;

            struct JSTuple
            {
                Ref<JSScript> script;
            };
            struct JSInvokeTuple
            {
                const std::string& method;
            };

            JSScript::JSScript(Ref<View> view, Ref<JSScriptSource> scriptSource)
                : Script(view, boost::static_pointer_cast<ScriptSource>(scriptSource)), context(nullptr)
            {
            }
            JSScript::~JSScript()
            {
                // Clean script references
                Ref<JSScriptSource> scriptSource = boost::dynamic_pointer_cast<JSScriptSource>(scriptSource);
                scriptSource->CleanScripts();
            }

            void JSScript::PrepareTimeout(size_t t)
            {
                startTime = clock() / (CLOCKS_PER_SEC / 1000);
                maxTime = t;
            }

            bool JSScript::Initialize()
            {
                Script::Initialize();

                try
                {
                    // Initialize javascript runtime
                    context = std::unique_ptr<duk_context, ContextDeleter>(
                        duk_create_heap(nullptr, nullptr, nullptr, this, nullptr));
                    if (context != nullptr)
                    {
                        duk_context* context = GetDuktapeContext();

                        // Prepare context
                        {

                            // Import utils module
                            JSUtils::duk_import(context);
                            JSValue::duk_import(context);

                            // Import main module
                            JSRoom::duk_import(context);
                            JSDevice::duk_import(context);

                            // Load script source
                            std::string data = scriptSource->GetContent();
                            duk_push_lstring(context, (const char*)data.c_str(), data.size());

                            std::string name = scriptSource->GetName();
                            duk_push_lstring(context, (const char*)name.c_str(), name.size());

                            // Compile
                            duk_compile(context, 0); // [ func ]

                            // Prepare timout
                            PrepareTimeout(5000); // 5 seconds

                            // Call script
                            duk_call(context, 0); // [ undefined ]
                            duk_pop(context);     // [ ]
                        }

                        // Initialize
                        {
                            // Push global object
                            duk_push_global_object(context); // [ object ]

                            // Initialize attributes
                            InitializeAttributes();

                            // Initialize properties
                            InitializeProperties();

                            // Initialize methods
                            InitializeMethods();

                            // Initialize events
                            InitializeEvents();

                            // Pop global
                            duk_pop(context); // [ ]
                        }

                        // Call initialize
                        if (duk_get_global_lstring(context, InitializeFunction, InitializeFunctionSize)) // [ func ]
                        {
                            // Prepare timeout
                            PrepareTimeout(500); // 500ms

                            // Call function
                            duk_call(context, 0); // [ bool ]
                            duk_pop(context);     // [ ]
                        }
                    }
                    else
                        return false;
                }
                catch (std::runtime_error e)
                {
                    LOG_WARNING("Duktape: {0}", e.what());

                    context = nullptr;
                }

                return true;
            }

            void JSScript::InitializeAttributes()
            {
                duk_context* context = GetDuktapeContext();
                assert(context != nullptr);

                DUK_TEST_ENTER(context);

                // Get attributes object
                duk_get_prop_lstring(context, -1, AttributesProperty, AttributesPropertySize); // [ object ]

                if (duk_is_object(context, -1))
                {
                    // Iterate over attributes
                    duk_enum(context, -1, 0); // [ object enum ]

                    while (duk_next(context, -1, 1)) // [ object enum key value ]
                    {
                        // Get name
                        size_t nameLength;
                        const char* nameStr = duk_to_lstring(context, -2, &nameLength);
                        std::string name = std::string(nameStr, nameLength);

                        // Convert attribute to json
                        duk_json_encode(context, -1); // [ object enum key json ]

                        size_t jsonLength;
                        const char* jsonStr = duk_to_lstring(context, -1, &jsonLength);

                        // Add attribute
                        {
                            rapidjson::Document document;

                            document.Parse(jsonStr, jsonLength);

                            // Add attribute to list
                            if (!document.HasParseError())
                            {
                                // Insert attribute
                                attributeList[name] = std::move(document);
                            }
                            else
                                duk_error(context, DUK_ERR_ERROR, "Invalid json.");
                        }

                        // Pop json, and key
                        duk_pop_2(context); // [ object enum ]
                    }

                    // Pop enum
                    duk_pop(context); // [ object ]
                }

                // Pop object
                duk_pop(context); // [ ]

                DUK_TEST_LEAVE(context, 0);

                attributeList.compact();
            }
            void JSScript::InitializeProperties()
            {
                duk_context* context = GetDuktapeContext();
                assert(context != nullptr);

                propertyByIDList.clear();

                DUK_TEST_ENTER(context);

                // Get properties object
                duk_get_prop_lstring(context, -1, PropertiesProperty, PropertiesPropertySize); // [ object ]

                if (duk_is_object(context, -1)) // [ object ]
                {
                    // Iterate over properties
                    duk_enum(context, -1, 0); // [ object enum ]

                    while (duk_next(context, -1, 1)) // [ object enum key value ]
                    {
                        // Get name
                        size_t nameLength;
                        const char* nameStr = duk_to_lstring(context, -2, &nameLength);
                        std::string name = std::string(nameStr, nameLength);

                        // Get type
                        size_t typeLength;
                        const char* typeStr = duk_get_lstring(context, -1, &typeLength);
                        ValueType type = ParseValueType(std::string_view(typeStr, typeLength));

                        uint32_t index;

                        // Add property
                        Ref<Value> property = Value::Create(type);
                        if (property != nullptr)
                        {
                            // Insert property
                            propertyList[name] = property;

                            // Insert property id reference
                            index = (uint32_t)propertyByIDList.size();
                            if (index >= 32)
                                duk_error(context, DUK_ERR_ERROR, "Too many properties.");

                            propertyByIDList.push_back(property);
                        }

                        // Pop value
                        duk_pop(context); // [ object enum key ]

                        // Push getter function
                        duk_push_c_function(context, JSScript::duk_get_property, 0); // [ object enum key c_func ]
                        duk_set_magic(context, -1, index);

                        // Push setter function
                        duk_push_c_function(context, JSScript::duk_set_property,
                                            1); // [ object enum key c_func c_func ]
                        duk_set_magic(context, -1, index);

                        // Set properties
                        duk_def_prop(context, -5,
                                     DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER | DUK_DEFPROP_CLEAR_EC |
                                         DUK_DEFPROP_FORCE); // [ object enum ]
                    }

                    // Pop enum
                    duk_pop(context); // [ object ]
                }

                // Pop object
                duk_pop(context); // [ ]

                DUK_TEST_LEAVE(context, 0);

                propertyByIDList.shrink_to_fit();
            }
            void JSScript::InitializeMethods()
            {
                duk_context* context = GetDuktapeContext();
                assert(context != nullptr);

                DUK_TEST_ENTER(context);

                // Get stash
                duk_push_global_stash(context); // [ stash ]

                // Iterate over methods
                duk_enum(context, -2, 0); // [ stash enum ]

                while (duk_next(context, -1, 1)) // [ stash enum key value ]
                {
                    // Get name
                    size_t nameLength;
                    const char* nameStr = duk_to_lstring(context, -2, &nameLength);
                    std::string name = std::string(nameStr, nameLength);

                    if (duk_is_ecmascript_function(context, -1) && name.size() > 0 &&
                        name[0] != '_') // [ stash enum key func ]
                    {
                        // Set method
                        duk_def_prop(context, -4,
                                     DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_CLEAR_WEC |
                                         DUK_DEFPROP_FORCE); // [ stash object enum ]
                    }
                    else
                    {
                        // Pop value, and key
                        duk_pop_2(context); // [ stash enum ]
                    }
                }

                // Pop enum, and stash
                duk_pop_2(context); // [ stash enum ]

                DUK_TEST_LEAVE(context, 0);
            }
            void JSScript::InitializeEvents()
            {
                duk_context* context = GetDuktapeContext();
                assert(context != nullptr);

                eventByIDList.clear();

                DUK_TEST_ENTER(context);

                // Get events object
                duk_get_prop_lstring(context, -1, EventsProperty, EventsPropertySize); // [ object ]

                if (duk_is_object(context, -1)) // [ object ]
                {
                    // Iterate over events
                    duk_enum(context, -1, 0); // [ object enum ]

                    while (duk_next(context, -1, 0)) // [ object enum key ]
                    {
                        size_t nameLength;
                        const char* nameStr = duk_to_lstring(context, -1, &nameLength);
                        std::string name = std::string(nameStr, nameLength);

                        uint32_t index;

                        // Add event
                        Ref<Event> event = Event::Create();
                        if (event != nullptr)
                        {
                            // Insert event
                            eventList[name] = event;

                            // Insert event id reference
                            index = (uint32_t)eventByIDList.size();
                            if (index >= 32)
                                duk_error(context, DUK_ERR_ERROR, "Too many events.");

                            eventByIDList.push_back(event);
                        }

                        // Push function
                        duk_push_c_function(context, JSScript::duk_invoke_event, 0); // [ object enum key c_func ]
                        duk_set_magic(context, -1, index);

                        // Set event
                        duk_def_prop(context, -4,
                                     DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_CLEAR_WEC |
                                         DUK_DEFPROP_FORCE); // [ object enum ]
                    }

                    // Pop enum
                    duk_pop(context); // [ object ]
                }

                // Pop object
                duk_pop(context); // [ ]

                DUK_TEST_LEAVE(context, 0);

                eventByIDList.shrink_to_fit();
            }
            void JSScript::InitializeControllers()
            {
                //                 assert(context != nullptr);

                //                 duk_context* c = context.get();

                // #ifndef NDEBUG
                //                 duk_idx_t top1 = duk_get_top(c);
                // #endif

                //                 // Get events object
                //                 duk_get_global_lstring(c, "controllers", 11); // [ object ]

                //                 if (duk_is_object(c, -1))
                //                 {
                //                     // Iterate over every property in 'controllers'
                //                     duk_enum(c, -1, 0); // [ object enum ]

                //                     while (duk_next(c, -1, 0)) // [ object enum key ]
                //                     {
                //                         size_t nameLength;
                //                         const char* nameStr = duk_to_lstring(c, -1, &nameLength);
                //                         std::string name = std::string(nameStr, nameLength);

                //                         // Read property from 'events'
                //                         duk_get_prop(c, -3); // [ object enum func ]

                //                         if (duk_is_function(c, -1))
                //                         {
                //                             // Safe event as hidden variable
                //                             std::string function_name = DUK_EVENT_FUNCTION_NAME(name);
                //                             duk_put_global_lstring(c, function_name.data(), function_name.size()); //
                //                             [ object enum ]

                //                             // Add event
                //                             Ref<Method> event = Method::Create(name, &JSScript::InvokeImpl);
                //                             if (event != nullptr)
                //                             {
                //                                 // Insert property
                //                                 eventList[name] = event;
                //                             }
                //                         }
                //                         else
                //                             duk_pop(c); // [ object enum ]
                //                     }

                //                     // Pop enum
                //                     duk_pop(c); // [ object ]
                //                 }

                //                 // Pop object
                //                 duk_pop(c); // [ ]

                // #ifndef NDEBUG
                //                 duk_idx_t top2 = duk_get_top(c);

                //                 // Check stack before and after
                //                 assert(top1 == top2);
                // #endif
            }

            bool JSScript::Invoke(const std::string& name, Ref<Value> parameter)
            {
                duk_context* context = GetDuktapeContext();
                if (context != nullptr)
                {
                    try
                    {
                        // Push stash
                        duk_push_global_stash(context);

                        // Get event
                        if (duk_get_prop_lstring(context, -1, name.data(), name.size())) // [ stash func ]
                        {
                            // Push parameter
                            JSValue::duk_new_value(context, parameter); // [ stash func value ]

                            // Prepare timout
                            PrepareTimeout(100); // 100ms

                            // Call event function
                            duk_call(context, 1); // [ stash bool ]

                            // Get result
                            bool result = duk_to_boolean(context, -1);

                            // Pop bool, and stash
                            duk_pop_2(context); // [ ]

                            return result;
                        }
                        else
                        {
                            // Pop undefined, and stash
                            duk_pop_2(context); // [ ]

                            return false;
                        }
                    }
                    catch (std::runtime_error e)
                    {
                        // TODO Error message duk_safe_to_string(context, -1);
                        LOG_WARNING("Duktape: {0}", e.what());

                        return false;
                    }
                }
                else
                    return false;
            }

            duk_ret_t JSScript::duk_get_property(duk_context* context)
            {
                JSScript* script = (JSScript*)duk_get_user_data(context);

                uint32_t index = duk_get_current_magic(context);
                if (index < script->propertyByIDList.size())
                {
                    Ref<Value> property = script->propertyByIDList[index];
                    assert(property != nullptr);

                    JSValue::duk_new_value(context, property);

                    return 1; // [ value ]
                }
                else
                {
                    // Error
                    return DUK_RET_ERROR;
                }
            }
            duk_ret_t JSScript::duk_set_property(duk_context* context)
            {
                JSScript* script = (JSScript*)duk_get_user_data(context);

                uint32_t index = duk_get_current_magic(context);
                if (index < script->propertyByIDList.size() && duk_get_top(context) == 1)
                {
                    Ref<Value> property = script->propertyByIDList[index];
                    assert(property != nullptr);

                    JSValue::duk_get_value(context, -1, property);

                    return 0;
                }
                else
                {
                    // Error
                    return DUK_RET_ERROR;
                }
            }

            duk_ret_t JSScript::duk_invoke_event(duk_context* context)
            {
                JSScript* script = (JSScript*)duk_get_user_data(context);

                uint32_t index = duk_get_current_magic(context);
                if (index < script->eventByIDList.size() && duk_get_top(context) == 1)
                {
                    Ref<Event> event = script->eventByIDList[index];
                    assert(event != nullptr);

                    event->PostInvoke(nullptr);

                    return 0;
                }
                else
                {
                    // Error
                    return DUK_RET_ERROR;
                }
            }
        }
    }
}