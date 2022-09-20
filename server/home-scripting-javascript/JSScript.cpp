#include "JSScript.hpp"
#include "JSScriptSource.hpp"
#include <home-scripting/utils/Event.hpp>
#include <home-scripting/utils/Value.hpp>

#include "main/JSDevice.hpp"
#include "main/JSRoom.hpp"

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

duk_ret_t duk_print(duk_context* context)
{
    std::stringstream ss;

    size_t count = duk_get_top(context);
    for (size_t index = 0; index < count; index++)
    {
        size_t valueLength;
        const char* valueStr = duk_safe_to_lstring(context, index, &valueLength);

        ss << std::string_view(valueStr, valueLength);
    }

    LOG_INFO("Duktape Log: {0}", ss.str());

    return 0;
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

            JSScript::JSScript(Ref<View> view, Ref<JSScriptSource> source)
                : Script(view, boost::static_pointer_cast<ScriptSource>(source)), context(nullptr)
            {
            }
            JSScript::~JSScript()
            {
            }

            void JSScript::PrepareTimeout(size_t maxTime)
            {
                startTime = clock() / (CLOCKS_PER_SEC / 1000);
                maxTime = maxTime;
            }

            bool JSScript::Initialize()
            {
                // Check if compilation stage is needed
                size_t cs = scriptSource->GetChecksum();
                if (cs != checksum || context == nullptr)
                {
                    // Terminate script before reinitializing it
                    if (context != nullptr)
                        Terminate();

                    try
                    {
                        // Initialize javascript runtime
                        context = std::unique_ptr<duk_context, ContextDeleter>(
                            duk_create_heap(nullptr, nullptr, nullptr, this, nullptr));
                        if (context == nullptr)
                        {
                            checksum = 0;
                            return false;
                        }

                        duk_context* c = GetDuktapeContext();

                        // Prepare context
                        {
                            JSTuple tuple = {boost::dynamic_pointer_cast<JSScript>(shared_from_this())};
                            if (duk_safe_call(c, JSScript::InitializeSafe, (void*)&tuple, 0, 1) != DUK_EXEC_SUCCESS)
                            {
                                // TODO Error message duk_safe_to_string(context, -1);
                                duk_pop(c);

                                return false;
                            }
                            else
                                duk_pop(c);
                        }

                        // Call initialize function
                        if (duk_get_global_lstring(c, "initialize", 10)) // [ func ]
                        {
                            // Prepare timout
                            PrepareTimeout(5000); // 5 seconds

                            // Call function
                            duk_call(c, 0); // [ bool ]
                            duk_pop(c);
                        }

                        // Set checksum to prevent recompilation
                        checksum = cs;
                    }
                    catch (std::exception e)
                    {
                        LOG_WARNING("Duktape: {0}", e.what());

                        context = nullptr;
                        checksum = 0;
                    }
                }

                return true;
            }

            duk_ret_t JSScript::InitializeSafe(duk_context* context, void* udata)
            {
                auto& [script] = *(JSTuple*)udata;

                // Import modules
                {
                    // Push global object
                    duk_push_global_object(context);

                    // Register methods
                    static const duk_function_list_entry methods[] = {
                        {
                            "print",
                            duk_print,
                            DUK_VARARGS,
                        },
                        {
                            "createTimer",
                            JSScript::CreateTimer,
                            2,
                        },
                        {
                            "createInterval",
                            JSScript::CreateInterval,
                            2,
                        },
                        {nullptr, nullptr, 0},
                    };

                    duk_put_function_list(context, -1, methods);

                    // Pop global object
                    duk_pop(context);
                }

                // Import main
                JSRoom::Import(context);
                JSDevice::Import(context);

                // Load script source
                std::string data = script->scriptSource->GetContent();
                duk_push_lstring(context, (const char*)data.c_str(), data.size());

                std::string name = script->scriptSource->GetName();
                duk_push_lstring(context, (const char*)name.c_str(), name.size());

                // Compile
                duk_compile(context, 0);

                // Prepare timout
                script->PrepareTimeout(5000); // 5 seconds

                // Call script
                duk_call(context, 0);
                duk_pop(context);

                // Clear interface
                script->attributeList.clear();
                script->propertyList.clear();
                script->propertyByIDList.clear();
                script->methodList.clear();

                // Initialize interface
                script->InitializeAttributes();
                script->InitializeProperties();
                script->InitializeMethods();

                return DUK_EXEC_SUCCESS;
            }

            void JSScript::InitializeAttributes()
            {
                duk_context* c = GetDuktapeContext();
                assert(c != nullptr);

#ifndef NDEBUG
                duk_idx_t top1 = duk_get_top(c);
#endif

                duk_get_global_lstring(c, "attributes", 10); // [ object ]

                if (duk_is_object(c, -1))
                {
                    // Iterate over every property in 'attributes'
                    duk_enum(c, -1, 0); // [ object enum ]

                    while (duk_next(c, -1, 0)) // [ object enum key ]
                    {
                        size_t nameLength;
                        const char* nameStr = duk_to_lstring(c, -1, &nameLength);
                        std::string name = std::string(nameStr, nameLength);

                        // Read property from 'attributes'
                        duk_get_prop(c, -3); // [ object enum object ]

                        // Convert attribute to json
                        duk_json_encode(c, -1); // [ object enum json ]

                        size_t jsonLength;
                        const char* jsonStr = duk_to_lstring(c, -1, &jsonLength);

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
                        duk_pop(c); // [ object enum ]
                    }

                    // Pop enum
                    duk_pop(c); // [ object ]
                }

                // Pop object
                duk_pop(c); // [ ]

#ifndef NDEBUG
                duk_idx_t top2 = duk_get_top(c);

                // Check stack before and after
                assert(top1 == top2);
#endif

                attributeList.compact();
            }
            void JSScript::InitializeProperties()
            {
                duk_context* c = GetDuktapeContext();
                assert(c != nullptr);

#ifndef NDEBUG
                duk_idx_t top1 = duk_get_top(c);
#endif

                duk_get_global_lstring(c, "properties", 10); // [ object ]

                if (duk_is_object(c, -1))
                {
                    // Iterate over every property in 'properties'
                    duk_enum(c, -1, 0); // [ object enum ]

                    while (duk_next(c, -1, 0)) // [ object enum key ]
                    {
                        size_t nameLength;
                        const char* nameStr = duk_to_lstring(c, -1, &nameLength);
                        std::string name = std::string(nameStr, nameLength);

                        duk_dup_top(c); // [ object enum key key ]

                        // Read property from 'properties'
                        duk_get_prop(c, -4); // [ object enum key object ]

                        size_t typeLength;
                        const char* typeStr = duk_to_lstring(c, -1, &typeLength);
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
                                duk_error(c, DUK_ERR_ERROR, "Too many properties.");

                            propertyByIDList.push_back(property);
                        }

                        // Pop object
                        duk_pop(c); // [ object enum key ]

                        // Push function
                        duk_push_c_function(c, JSScript::GetProperty, 0); // [ object enum key c_func ]
                        duk_set_magic(c, -1, index);
                        duk_push_c_function(c, JSScript::SetProperty, 1); // [ object enum key c_func c_func ]
                        duk_set_magic(c, -1, index);

                        // Set property getter and setter
                        duk_def_prop(c, -5,
                                     DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER | DUK_DEFPROP_FORCE |
                                         DUK_DEFPROP_HAVE_ENUMERABLE |
                                         DUK_DEFPROP_HAVE_CONFIGURABLE); // [ object enum ]
                    }

                    // Pop enum
                    duk_pop(c); // [ object ]
                }

                // Pop object
                duk_pop(c); // [ ]

#ifndef NDEBUG
                duk_idx_t top2 = duk_get_top(c);

                // Check stack before and after
                assert(top1 == top2);
#endif

                propertyByIDList.shrink_to_fit();
            }
            void JSScript::InitializeMethods()
            {
                duk_context* c = GetDuktapeContext();
                assert(c != nullptr);

#ifndef NDEBUG
                duk_idx_t top1 = duk_get_top(c);
#endif

                // Get methods object
                duk_get_global_lstring(c, "methods", 7); // [ object ]

                if (duk_is_object(c, -1))
                {
                    // Iterate over every property in 'methods'
                    duk_enum(c, -1, 0); // [ object enum ]

                    while (duk_next(c, -1, 0)) // [ object enum key ]
                    {
                        size_t nameLength;
                        const char* nameStr = duk_to_lstring(c, -1, &nameLength);
                        std::string name = std::string(nameStr, nameLength);

                        // Read property from 'methods'
                        duk_get_prop(c, -3); // [ object enum func ]

                        if (duk_is_function(c, -1))
                        {
                            // Safe event as hidden variable
                            std::string function_name = DUK_EVENT_FUNCTION_NAME(name);
                            duk_put_global_lstring(c, function_name.data(), function_name.size()); // [ object enum ]

                            // Add method
                            Ref<Method> method = Method::Create<JSScript>(name, shared_from_this(), &JSScript::Invoke);
                            if (method != nullptr)
                                methodList[name] = method;
                        }
                        else
                            duk_pop(c); // [ object enum ]
                    }

                    // Pop enum
                    duk_pop(c); // [ object ]
                }

                // Pop object
                duk_pop(c); // [ ]

#ifndef NDEBUG
                duk_idx_t top2 = duk_get_top(c);

                // Check stack before and after
                assert(top1 == top2);
#endif
            }
            void JSScript::InitializeEvents()
            {
                duk_context* c = GetDuktapeContext();
                assert(c != nullptr);

#ifndef NDEBUG
                duk_idx_t top1 = duk_get_top(c);
#endif

                // Get events object
                duk_get_global_lstring(c, "events", 6); // [ object ]

                if (duk_is_object(c, -1))
                {
                    // Iterate over every property in 'events'
                    duk_enum(c, -1, 0); // [ object enum ]

                    while (duk_next(c, -1, 0)) // [ object enum key ]
                    {
                        size_t nameLength;
                        const char* nameStr = duk_to_lstring(c, -1, &nameLength);
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
                                duk_error(c, DUK_ERR_ERROR, "Too many events.");

                            eventByIDList.push_back(event);
                        }

                        // Pop object
                        duk_pop(c); // [ object enum key ]

                        // Push function
                        duk_push_c_function(c, JSScript::InvokeEvent, 0); // [ object enum key c_func ]
                        duk_set_magic(c, -1, index);

                        // Set event
                        duk_def_prop(c, -4,
                                     DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_FORCE | DUK_DEFPROP_HAVE_ENUMERABLE |
                                         DUK_DEFPROP_HAVE_CONFIGURABLE); // [ object enum ]
                    }

                    // Pop enum
                    duk_pop(c); // [ object ]
                }

                // Pop object
                duk_pop(c); // [ ]

#ifndef NDEBUG
                duk_idx_t top2 = duk_get_top(c);

                // Check stack before and after
                assert(top1 == top2);
#endif
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
                assert(context != nullptr);

                try
                {
                    // Get events object
                    std::string function_name = DUK_EVENT_FUNCTION_NAME(name);
                    if (duk_get_global_lstring(context, function_name.data(), function_name.size())) // [ func ]
                    {
                        // Prepare timout
                        PrepareTimeout(5000); // 5 seconds

                        // Call event function
                        duk_call(context, 0); // [ bool ]

                        // Get result
                        bool result = duk_to_boolean(context, -1);
                        duk_pop(context);

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

            struct TimerTask
            {
                const WeakRef<JSScript> script;
                boost::shared_ptr<boost::asio::deadline_timer> timer;
                const std::string method;
                const uint64_t interval;
            };

            void TimerHandler(const boost::system::error_code& ec, TimerTask task)
            {
                Ref<JSScript> script = task.script.lock();
                if (script != nullptr && !ec)
                {
                    try
                    {
                        duk_context* context = script->GetDuktapeContext();
                        // Call call function
                        if (duk_get_global_lstring(context, task.method.data(), task.method.size())) // [ func ]
                        {
                            // Prepare timout
                            script->PrepareTimeout(5000); // 5 seconds

                            // Call function
                            duk_call(context, 0); // [ bool ]

                            if (duk_to_boolean(context, -1))
                            {
                                // Restart timer if true was returned
                                boost::asio::deadline_timer& timer = *task.timer;
                                timer.expires_from_now(boost::posix_time::seconds(task.interval));
                                timer.async_wait(boost::bind(&TimerHandler, boost::placeholders::_1, std::move(task)));
                            }

                            duk_pop(context);
                        }
                    }
                    catch (std::runtime_error e)
                    {
                    }
                }
            }
            duk_ret_t JSScript::CreateTimer(duk_context* context)
            {
                JSScript* script = (JSScript*)duk_get_user_data(context);

                // Expect [ string number ]
                if (duk_get_top(context) == 2 && duk_is_string(context, -2) && duk_is_number(context, -1))
                {
                    // Get method name
                    size_t methodLength;
                    const char* methodStr = duk_get_lstring(context, -2, &methodLength);
                    std::string method = std::string(methodStr, methodLength);

                    // Get timer interval
                    duk_uint_t interval = duk_get_uint(context, -1);

                    // Pop number and string
                    duk_pop_2(context);

                    Ref<Worker> worker = Worker::GetInstance();
                    assert(worker != nullptr);

                    // Create timer task
                    TimerTask task = TimerTask{
                        .script = Ref<JSScript>(boost::dynamic_pointer_cast<JSScript>(script->shared_from_this())),
                        .timer = boost::make_shared<boost::asio::deadline_timer>(worker->GetContext()),
                        .method = std::move(method),
                        .interval = interval,
                    };

                    // Start timer
                    boost::asio::deadline_timer& timer = *task.timer;
                    timer.expires_from_now(boost::posix_time::seconds(task.interval));
                    timer.async_wait(boost::bind(&TimerHandler, boost::placeholders::_1, std::move(task)));

                    return 0;
                }
                else
                {
                    // Error
                    return DUK_RET_ERROR;
                }
            }

            void IntervalHandler(const boost::system::error_code& ec, TimerTask task)
            {
                Ref<JSScript> script = task.script.lock();
                if (script != nullptr && !ec)
                {
                    try
                    {
                        duk_context* context = script->GetDuktapeContext();

                        // Call call function
                        if (duk_get_global_lstring(context, task.method.data(), task.method.size())) // [ func ]
                        {
                            // Prepare timout
                            script->PrepareTimeout(5000); // 5 seconds

                            // Call function
                            duk_call(context, 0); // [ bool ]
                            duk_pop(context);
                        }
                    }
                    catch (std::runtime_error e)
                    {
                    }
                }
            }
            duk_ret_t JSScript::CreateInterval(duk_context* context)
            {
                JSScript* script = (JSScript*)duk_get_user_data(context);

                // Expect [ string number ]
                if (duk_get_top(context) == 2 && duk_is_string(context, -2) && duk_is_number(context, -1))
                {
                    // Get method name
                    size_t methodLength;
                    const char* methodStr = duk_get_lstring(context, -2, &methodLength);
                    std::string method = std::string(methodStr, methodLength);

                    // Get timer interval
                    duk_uint_t interval = duk_get_uint(context, -1);

                    Ref<Worker> worker = Worker::GetInstance();
                    assert(worker != nullptr);

                    // Create timer task
                    TimerTask task = TimerTask{
                        .script = Ref<JSScript>(boost::dynamic_pointer_cast<JSScript>(script->shared_from_this())),
                        .timer = boost::make_shared<boost::asio::deadline_timer>(worker->GetContext()),
                        .method = std::move(method),
                        .interval = interval,
                    };

                    // Start timer
                    boost::asio::deadline_timer& timer = *task.timer;
                    timer.expires_from_now(boost::posix_time::seconds(task.interval));
                    timer.async_wait(boost::bind(&IntervalHandler, boost::placeholders::_1, std::move(task)));

                    return 0;
                }
                else
                {
                    // Error
                    return DUK_RET_ERROR;
                }
            }

            duk_ret_t JSScript::GetProperty(duk_context* context)
            {
                JSScript* script = (JSScript*)duk_get_user_data(context);

                uint32_t index = duk_get_current_magic(context);
                if (index < script->propertyByIDList.size())
                {
                    Ref<Value> property = script->propertyByIDList[index];
                    assert(property != nullptr);

                    switch (property->GetType())
                    {
                    case ValueType::kBooleanType:
                        duk_push_boolean(context, property->GetBoolean());
                        return 1;
                    case ValueType::kIntegerType:
                        duk_push_int(context, property->GetInteger());
                        return 1;
                    case ValueType::kNumberType:
                        duk_push_number(context, property->GetNumber());
                        return 1;
                    case ValueType::kStringType:
                    {
                        std::string string = property->GetString();
                        duk_push_lstring(context, string.data(), string.size());
                        return 1;
                    }
                    case ValueType::kEndpointType:
                    {
                        duk_push_object(context);

                        Endpoint endpoint = property->GetEndpoint();

                        duk_push_lstring(context, endpoint.host.data(), endpoint.host.size());
                        duk_put_prop_lstring(context, -2, "host", 4);

                        duk_push_int(context, endpoint.port);
                        duk_put_prop_lstring(context, -2, "port", 4);

                        return 1;
                    }
                    case ValueType::kColorType:
                    {
                        duk_push_object(context);

                        Color color = property->GetColor();

                        duk_push_int(context, color.red);
                        duk_put_prop_lstring(context, -2, "red", 3);

                        duk_push_int(context, color.green);
                        duk_put_prop_lstring(context, -2, "green", 5);

                        duk_push_int(context, color.blue);
                        duk_put_prop_lstring(context, -2, "blue", 4);

                        return 1;
                    }
                    default:
                        break;
                    }
                }

                // Error
                duk_push_null(context);
                return 1;
            }
            duk_ret_t JSScript::SetProperty(duk_context* context)
            {
                JSScript* script = (JSScript*)duk_get_user_data(context);

                uint32_t index = duk_get_current_magic(context);
                if (index < script->propertyByIDList.size() && duk_get_top(context) == 1)
                {
                    Ref<Value> property = script->propertyByIDList[index];
                    assert(property != nullptr);

                    switch (property->GetType())
                    {
                    case ValueType::kBooleanType:
                    {
                        property->SetBoolean(duk_to_boolean(context, -1));
                        duk_pop(context);

                        return 0;
                    }
                    case ValueType::kIntegerType:
                    {
                        property->SetInteger(duk_to_int(context, -1));
                        duk_pop(context);

                        return 0;
                    }
                    case ValueType::kNumberType:
                    {
                        property->SetNumber(duk_to_number(context, -1));
                        duk_pop(context);

                        return 0;
                    }
                    case ValueType::kStringType:
                    {
                        size_t length;
                        const char* string = duk_to_lstring(context, -1, &length);
                        property->SetString(std::string(string, length));
                        duk_pop(context);

                        return 0;
                    }
                    case ValueType::kEndpointType:
                    {
                        // Coerce value
                        duk_to_object(context, -1);

                        // Get each property
                        duk_get_prop_lstring(context, -1, "host", 4);
                        duk_get_prop_lstring(context, -2, "port", 4);

                        size_t hostLength;
                        const char* host = duk_to_lstring(context, -2, &hostLength);

                        // Set endpoint
                        Endpoint endpoint = {std::string(host, hostLength), (uint16_t)duk_to_int(context, -1)};

                        property->SetEndpoint(endpoint);

                        duk_pop_3(context);

                        return 0;
                    }
                    case ValueType::kColorType:
                    {
                        // Coerce value
                        duk_to_object(context, -1);

                        // Get each property
                        duk_get_prop_lstring(context, -1, "red", 3);
                        duk_get_prop_lstring(context, -2, "green", 5);
                        duk_get_prop_lstring(context, -3, "blue", 4);

                        // Set color
                        Color color = {(uint8_t)duk_to_int(context, -3), (uint8_t)duk_to_int(context, -2),
                                       (uint8_t)duk_to_int(context, -1)};
                        property->SetColor(color);

                        duk_pop_n(context, 4);

                        return 0;
                    }
                    default:
                        break;
                    }
                }

                // Error
                return 0;
            }

            duk_ret_t JSScript::InvokeEvent(duk_context* context)
            {
                JSScript* script = (JSScript*)duk_get_user_data(context);

                uint32_t index = duk_get_current_magic(context);
                if (index < script->eventByIDList.size() && duk_get_top(context) == 1)
                {
                    Ref<Event> event = script->eventByIDList[index];
                    assert(event != nullptr);

                    event->PostInvoke(nullptr);
                }

                // Error
                return 0;
            }

            bool JSScript::Terminate()
            {
                duk_context* c = GetDuktapeContext();

                if (c != nullptr)
                {
                    // Call terminate function
                    if (duk_get_global_lstring(c, "terminate", 9)) // [ func ]
                    {
                        // Prepare timout
                        PrepareTimeout(5000); // 5 seconds

                        // Call function
                        duk_call(c, 0); // [ bool ]
                        duk_pop(c);
                    }
                }

                // Destroy javascript runtime
                context = nullptr;

                return true;
            }

            duk_ret_t JSScript::TerminateSafe(duk_context* context, void* udata)
            {
                return DUK_EXEC_SUCCESS;
            }
        }
    }
}