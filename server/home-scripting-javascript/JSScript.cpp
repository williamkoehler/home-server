#include "JSScript.hpp"
#include "JSScriptSource.hpp"
#include <home-scripting/Value.hpp>
#include <home-scripting/interface/Event.hpp>

#include "JSUtils.hpp"
#include "wrapper/JSValue.hpp"
#include "wrapper/main/JSHome.hpp"

#include "wrapper/literals.hpp"

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

#define MIN_UPDATE_TIME (size_t)(5)

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

            JSScript::JSScript(const Ref<View>& view, const Ref<JSScriptSource>& scriptSource)
                : Script(view, boost::static_pointer_cast<ScriptSource>(scriptSource)), context(nullptr)
            {
            }
            JSScript::~JSScript()
            {
                // Clean script references
                Ref<JSScriptSource> s = boost::dynamic_pointer_cast<JSScriptSource>(scriptSource);
                s->CleanScripts();
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
                            duk_import_utils(context);
                            duk_import_value(context);

                            // Import main module
                            duk_import_home(context);

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
                        if (duk_get_global_lstring(context, INITIALIZE_FUNCTION, INITIALIZE_FUNCTION_SIZE)) // [ func ]
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
                duk_get_prop_lstring(context, -1, ATTRIBUTES_PROPERTY, ATTRIBUTES_PROPERTY_SIZE); // [ object ]

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
                                attributeMap[name] = std::move(document);
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

                attributeMap.compact();
            }
            void JSScript::InitializeProperties()
            {
                duk_context* context = GetDuktapeContext();
                assert(context != nullptr);

                propertyMap.clear();

                DUK_TEST_ENTER(context);

                // Get properties object
                duk_get_prop_lstring(context, -1, PROPERTIES_PROPERTY, PROPERTIES_PROPERTY_SIZE); // [ object ]

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

                        // Add property
                        propertyMap.insert_or_assign(name, Value(type));

                        // Check property count
                        if (propertyMap.size() > 32)
                            duk_error(context, DUK_ERR_ERROR, "Too many properties.");

                        // Pop value, and key
                        duk_pop_2(context); // [ object enum ]
                    }

                    // Pop enum
                    duk_pop(context); // [ object ]
                }

                // Pop object
                duk_pop(context); // [ ]

                // Replace properties object by proxy
                duk_push_lstring(context, PROPERTIES_PROPERTY, PROPERTIES_PROPERTY_SIZE); // [ key ]
                duk_push_object(context);                                                 // [ key target ]
                duk_push_object(context);                                                 // [ key target handler ]

                static const duk_function_list_entry methods[] = {
                    {"get", duk_get_property, 2},
                    {"set", duk_set_property, 3},
                    {nullptr, nullptr, 0},
                };

                duk_put_function_list(context, -1, methods); // [ key target handler ]

                duk_push_proxy(context, 0); // [ key proxy ]
                duk_def_prop(context, -2,
                             DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_E | DUK_DEFPROP_CLEAR_WC |
                                 DUK_DEFPROP_FORCE); // [ key proxy ]

                DUK_TEST_LEAVE(context, 0);
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

                DUK_TEST_ENTER(context);

                // Get events object
                duk_get_prop_lstring(context, -1, EVENTS_PROPERTY, EVENTS_PROPERTY_SIZE); // [ object ]

                if (duk_is_object(context, -1)) // [ object ]
                {
                    // Iterate over events
                    duk_enum(context, -1, 0); // [ object enum ]

                    while (duk_next(context, -1, 0)) // [ object enum key ]
                    {
                        size_t nameLength;
                        const char* nameStr = duk_to_lstring(context, -1, &nameLength);
                        std::string name = std::string(nameStr, nameLength);

                        // Add event
                        Event event = Event();
                        eventMap.emplace(name, std::move(event));

                        // Check event count
                        if (eventMap.size() > 32)
                            duk_error(context, DUK_ERR_ERROR, "Too many events.");

                        // Push function
                        duk_push_c_function(context, JSScript::duk_invoke_event, 0); // [ object enum key c_func ]

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

            Value JSScript::GetProperty(const std::string& name)
            {
                const robin_hood::unordered_node_map<std::string, Value>::const_iterator it = propertyMap.find(name);
                if (it != propertyMap.end())
                    return it->second;

                return Value();
            }
            void JSScript::SetProperty(const std::string& name, const Value& value)
            {
                const robin_hood::unordered_node_map<std::string, Value>::iterator it = propertyMap.find(name);
                if (it != propertyMap.end())
                    it->second.Assign(value);
            }

            bool JSScript::Invoke(const std::string& name, const Value& parameter)
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
                            duk_new_value(context, parameter); // [ stash func value ]

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

            void JSScript::JsonGetProperties(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator,
                                             PropertyFlags propertyFlags)
            {
                (void)propertyFlags;

                assert(output.IsObject());

                output.MemberReserve(propertyMap.size(), allocator);
                for (const auto& [name, property] : propertyMap)
                {
                    output.AddMember(rapidjson::Value(name.data(), name.size(), allocator), property.JsonGet(allocator),
                                     allocator);
                }
            }
            PropertyFlags JSScript::JsonSetProperties(const rapidjson::Value& input, PropertyFlags propertyFlags)
            {
                (void)propertyFlags;

                assert(input.IsObject());

                for (rapidjson::Value::ConstMemberIterator propertyIt = input.MemberBegin();
                     propertyIt != input.MemberEnd(); propertyIt++)
                {
                    std::string name = std::string(propertyIt->name.GetString(), propertyIt->name.GetStringLength());

                    const robin_hood::unordered_node_map<std::string, Value>::iterator it = propertyMap.find(name);
                    if (it != propertyMap.end())
                        it->second.JsonSet(propertyIt->value);
                }

                return true;
            }

            duk_ret_t JSScript::duk_get_property(duk_context* context)
            {
                // Expect [ object prop ]

                JSScript* script = (JSScript*)duk_get_user_data(context);

                size_t nameLength;
                const char* nameStr = duk_get_lstring(context, 1, &nameLength);
                std::string name = std::string(nameStr, nameLength);

                // Pop prop, and object
                duk_pop_2(context);

                // Get property
                const robin_hood::unordered_node_map<std::string, Value>::const_iterator it =
                    script->propertyMap.find(name);
                if (it != script->propertyMap.end())
                    duk_new_value(context, it->second);
                else
                    duk_push_undefined(context);

                return 1;
            }
            duk_ret_t JSScript::duk_set_property(duk_context* context)
            {
                // Expect [ object prop value ]

                JSScript* script = (JSScript*)duk_get_user_data(context);

                size_t nameLength;
                const char* nameStr = duk_get_lstring(context, 1, &nameLength);
                std::string name = std::string(nameStr, nameLength);

                // Set property
                const robin_hood::unordered_node_map<std::string, Value>::iterator it = script->propertyMap.find(name);
                if (it != script->propertyMap.end())
                    duk_get_value(context, 2, it->second);

                return 0;
            }

            duk_ret_t JSScript::duk_invoke_event(duk_context* context)
            {
                // Expect [ string value ]

                JSScript* script = (JSScript*)duk_get_user_data(context);

                size_t nameLength;
                const char* nameStr = duk_to_lstring(context, 0, &nameLength);
                std::string name = std::string(nameStr, nameLength);

                // Invoke event
                const robin_hood::unordered_node_map<std::string, Event>::const_iterator it =
                    script->eventMap.find(name);
                if (it != script->eventMap.end())
                    it->second.Invoke(duk_get_value(context, 1));

                return 0;
            }
        }
    }
}