#include "JSScript.hpp"
#include "JSScriptSource.hpp"
#include <home-scripting/utils/Event.hpp>
#include <home-scripting/utils/Value.hpp>

#include "JSUtils.hpp"
#include "main/JSDevice.hpp"
#include "main/JSRoom.hpp"
#include "utils/JSValue.hpp"

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

#define DUK_EVENT_FUNCTION_NAME(event) (DUK_HIDDEN_SYMBOL("event_") + event)

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
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

                            JSEndpoint::duk_import(context);
                            JSColor::duk_import(context);

                            // Import main module
                            JSRoom::duk_import(context);
                            JSDevice::duk_import(context);

                            // Load script source
                            std::string data = scriptSource->GetContent();
                            duk_push_lstring(context, (const char*)data.c_str(), data.size());

                            std::string name = scriptSource->GetName();
                            duk_push_lstring(context, (const char*)name.c_str(), name.size());

                            // Compile
                            duk_compile(context, 0);

                            // Prepare timout
                            PrepareTimeout(5000); // 5 seconds

                            // Call script
                            duk_call(context, 0);
                            duk_pop(context);

                            // Initialize attributes
                            InitializeAttributes();

                            // Initialize properties
                            propertyByIDList.clear();
                            InitializeProperties();

                            // Initialize methods
                            InitializeMethods();

                            // Initialize events
                            eventByIDList.clear();
                            InitializeEvents();
                        }

                        // Call initialize function
                        if (duk_get_global_lstring(context, "initialize", 10)) // [ func ]
                        {
                            // Prepare timeout
                            PrepareTimeout(2000); // 5 seconds

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

                duk_get_global_lstring(context, "attributes", 10); // [ object ]

                if (duk_is_object(context, -1))
                {
                    // Iterate over every property in 'attributes'
                    duk_enum(context, -1, 0); // [ object enum ]

                    while (duk_next(context, -1, 0)) // [ object enum key ]
                    {
                        size_t nameLength;
                        const char* nameStr = duk_to_lstring(context, -1, &nameLength);
                        std::string name = std::string(nameStr, nameLength);

                        // Read property from 'attributes'
                        duk_get_prop(context, -3); // [ object enum object ]

                        // Convert attribute to json
                        duk_json_encode(context, -1); // [ object enum json ]

                        size_t jsonLength;
                        const char* jsonStr = duk_to_lstring(context, -1, &jsonLength);

                        // Add attribute
                        {
                            rapidjson::Document document;

                            document.Parse(jsonStr, jsonLength);

                            // Add attribute to list
                            if (!document.HasParseError())
                                attributeList[name] = std::move(document);
                            else
                            {
                                LOG_ERROR("Parsing duktape json.");
                                LOG_CODE_MISSING("Add js error message.");
                            }
                        }

                        // Pop json
                        duk_pop(context); // [ object enum ]
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

                DUK_TEST_ENTER(context);

                duk_get_global_lstring(context, "properties", 10); // [ object ]

                if (duk_is_object(context, -1))
                {
                    // Iterate over every property in 'properties'
                    duk_enum(context, -1, 0); // [ object enum ]

                    while (duk_next(context, -1, 0)) // [ object enum key ]
                    {
                        size_t nameLength;
                        const char* nameStr = duk_to_lstring(context, -1, &nameLength);
                        std::string name = std::string(nameStr, nameLength);

                        duk_dup_top(context); // [ object enum key key ]

                        // Read property from 'properties'
                        duk_get_prop(context, -4); // [ object enum key object ]

                        size_t typeLength;
                        const char* typeStr = duk_to_lstring(context, -1, &typeLength);
                        std::string type = std::string(typeStr, typeLength);

                        uint32_t index;

                        // Add property
                        Ref<Value> property = Value::Create(ParseValueType(type));
                        if (property != nullptr)
                        {
                            // Insert property
                            propertyList[name] = property;

                            // Insert property id reference
                            index = (uint32_t)propertyByIDList.size();
                            if (index >= 64)
                                duk_error(context, DUK_ERR_ERROR, "Too many properties.");

                            propertyByIDList.push_back(property);
                        }

                        // Pop object
                        duk_pop(context); // [ object enum key ]

                        // Push function
                        duk_push_c_function(context, JSScript::duk_get_property, 0); // [ object enum key c_func ]
                        duk_set_magic(context, -1, index);
                        duk_push_c_function(context, JSScript::duk_set_property,
                                            1); // [ object enum key c_func c_func ]
                        duk_set_magic(context, -1, index);

                        // Set property getter and setter
                        duk_def_prop(context, -5,
                                     DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER | DUK_DEFPROP_FORCE |
                                         DUK_DEFPROP_HAVE_ENUMERABLE |
                                         DUK_DEFPROP_HAVE_CONFIGURABLE); // [ object enum ]
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

                // Get methods object
                duk_get_global_lstring(context, "methods", 7); // [ object ]

                if (duk_is_object(context, -1))
                {
                    // Iterate over every property in 'methods'
                    duk_enum(context, -1, 0); // [ object enum ]

                    while (duk_next(context, -1, 0)) // [ object enum key ]
                    {
                        size_t nameLength;
                        const char* nameStr = duk_to_lstring(context, -1, &nameLength);
                        std::string name = std::string(nameStr, nameLength);

                        // Read property from 'methods'
                        duk_get_prop(context, -3); // [ object enum func ]

                        if (duk_is_function(context, -1))
                        {
                            // Safe event as hidden variable
                            std::string function_name = DUK_EVENT_FUNCTION_NAME(name);
                            duk_put_global_lstring(context, function_name.data(),
                                                   function_name.size()); // [ object enum ]

                            // Add method
                            Ref<Method> method =
                                Method::Create<JSScript>(name, shared_from_this(), &JSScript::InvokeHandler);
                            if (method != nullptr)
                                methodList[name] = method;
                        }
                        else
                            duk_pop(context); // [ object enum ]
                    }

                    // Pop enum
                    duk_pop(context); // [ object ]
                }

                // Pop object
                duk_pop(context); // [ ]

                DUK_TEST_LEAVE(context, 0);
            }
            void JSScript::InitializeEvents()
            {
                duk_context* context = GetDuktapeContext();
                assert(context != nullptr);

                DUK_TEST_ENTER(context);

                // Get events object
                duk_get_global_lstring(context, "events", 6); // [ object ]

                if (duk_is_object(context, -1))
                {
                    // Iterate over every property in 'events'
                    duk_enum(context, -1, 0); // [ object enum ]

                    while (duk_next(context, -1, 0)) // [ object enum key ]
                    {
                        size_t nameLength;
                        const char* nameStr = duk_to_lstring(context, -1, &nameLength);
                        std::string name = std::string(nameStr, nameLength);

                        // duk_dup_top(c); // [ object enum key key ]

                        // // Read property from 'events'
                        // duk_get_prop(c, -4); // [ object enum key object ]

                        // size_t typeLength;
                        // const char* typeStr = duk_to_lstring(c, -1, &typeLength);
                        // std::string type = std::string(typeStr, typeLength);

                        uint32_t index;

                        // Add event
                        Ref<Event> event = Event::Create();
                        if (event != nullptr)
                        {
                            // Insert event
                            eventList[name] = event;

                            // Insert event id reference
                            index = (uint32_t)propertyByIDList.size();
                            if (index >= 64)
                                duk_error(context, DUK_ERR_ERROR, "Too many events.");

                            eventByIDList.push_back(event);
                        }

                        // Pop object
                        duk_pop(context); // [ object enum key ]

                        // Push function
                        duk_push_c_function(context, JSScript::duk_invoke_event, 0); // [ object enum key c_func ]
                        duk_set_magic(context, -1, index);

                        // Set event
                        duk_def_prop(context, -4,
                                     DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_FORCE | DUK_DEFPROP_HAVE_ENUMERABLE |
                                         DUK_DEFPROP_HAVE_CONFIGURABLE); // [ object enum ]
                    }

                    // Pop enum
                    duk_pop(context); // [ object ]
                }

                // Pop object
                duk_pop(context); // [ ]

                DUK_TEST_LEAVE(context, 0);
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

            bool JSScript::InvokeHandler(const std::string& name, Ref<Value> parameter)
            {
                duk_context* context = GetDuktapeContext();
                assert(context != nullptr);

                try
                {
                    // Get events object
                    std::string function_name = DUK_EVENT_FUNCTION_NAME(name);
                    if (duk_get_global_lstring(context, function_name.data(), function_name.size())) // [ func ]
                    {
                        // Push parameter
                        JSValue::duk_push_value(context, parameter); // [ value ]

                        // Prepare timout
                        PrepareTimeout(1000); // 5 seconds

                        // Call event function
                        duk_call(context, 1); // [ bool ]

                        // Get result
                        bool result = duk_to_boolean(context, -1);
                        duk_pop(context); // [ ]

                        return result;
                    }
                    else
                        return false;
                }
                catch (std::runtime_error e)
                {
                    // TODO Error message duk_safe_to_string(context, -1);
                    LOG_WARNING("Duktape: {0}", e.what());

                    return false;
                }
            }

            duk_ret_t JSScript::duk_get_property(duk_context* context)
            {
                JSScript* script = (JSScript*)duk_get_user_data(context);

                uint32_t index = duk_get_current_magic(context);
                if (index < script->propertyByIDList.size())
                {
                    Ref<Value> property = script->propertyByIDList[index];
                    assert(property != nullptr);

                    JSValue::duk_push_value(context, property);

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