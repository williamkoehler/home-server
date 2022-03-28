#include "JSScript.hpp"
#include "JSScriptSource.hpp"
#include "utils/JSEvent.hpp"
#include "utils/JSTimer.hpp"
#include <home-scripting/utils/Property.hpp>

extern "C"
{
    // Interrupt function called by the duktape engine
    // This allows to interruption of any javascript code that runs too long
    duk_ret_t duk_exec_timeout(void* udata)
    {
        return ((server::scripting::javascript::JSScript*)udata)->CheckTimeout();
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

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
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

            struct JSTuple
            {
                Ref<JSScript> script;
            };

            duk_ret_t JSScript::InitializeSafe(duk_context* context, void* udata)
            {
                auto& [script] = *(JSTuple*)udata;

                // Setup default variables
                duk_push_object(context);
                duk_put_global_lstring(context, "events", 6);

                duk_push_object(context);
                duk_put_global_lstring(context, "properties", 10);

                // Import modules
                {
                    // Push global object
                    duk_push_global_object(context);

                    // Register methods
                    static const duk_function_list_entry methods[] = {{"print", duk_print, DUK_VARARGS},
                                                                      {nullptr, nullptr, 0}};

                    duk_put_function_list(context, -1, methods);

                    // Pop global object
                    duk_pop(context);
                }

                // script->ImportModules();

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
                script->eventList.clear();

                // Initialize interface
                script->InitializeAttributes();
                script->InitializeProperties();
                script->InitializeEvents();
                script->InitializeTimers();

                return DUK_EXEC_SUCCESS;
            }

            void JSScript::InitializeAttributes()
            {
            }
            void JSScript::InitializeProperties()
            {
                assert(context != nullptr);

                duk_context* c = context.get();

                duk_get_global_lstring(c, "properties", 10);

                // Iterate over every property in 'properties'
                duk_enum(c, -1, 0);
                while (duk_next(c, -1, 0))
                {
                    size_t nameLength;
                    const char* nameStr = duk_to_lstring(c, -1, &nameLength);
                    std::string name = std::string(nameStr, nameLength);

                    // Read type
                    duk_dup_top(c);
                    duk_get_prop(c, -4);

                    size_t typeLength;
                    const char* typeStr = duk_to_lstring(c, -1, &typeLength);
                    std::string type = std::string(typeStr, typeLength);

                    uint32_t index = UINT32_MAX;

                    // Add property
                    Ref<Property> property = Property::Create(ParsePropertyType(type));
                    if (property != nullptr)
                    {
                        // Insert property
                        propertyList[name] = property;

                        // Insert property id reference
                        index = (uint32_t)propertyByIDList.size();
                        if (index >= UINT8_MAX)
                            duk_error(c, DUK_ERR_ERROR, "Too many properties");

                        propertyByIDList.push_back(property);
                    }

                    // Pop type
                    duk_pop(c);

                    // Push function
                    duk_push_c_function(c, JSScript::PropertyGetter, 0);
                    duk_set_magic(c, -1, index);
                    duk_push_c_function(c, JSScript::PropertySetter, 1);
                    duk_set_magic(c, -1, index);

                    // Set property getter and setter
                    duk_def_prop(c, -5,
                                 DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER | DUK_DEFPROP_FORCE |
                                     DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_HAVE_CONFIGURABLE);
                }

                // Pop enum
                duk_pop(c);

                propertyByIDList.shrink_to_fit();
            }
            void JSScript::InitializeEvents()
            {
                assert(context != nullptr);

                duk_context* c = context.get();

                // Get events object
                duk_get_global_lstring(c, "events", 6);

                // Iterate over every property in 'events'
                duk_enum(c, -1, 0);
                while (duk_next(c, -1, 0))
                {
                    size_t nameLength;
                    const char* nameStr = duk_to_lstring(c, -1, &nameLength);
                    std::string name = std::string(nameStr, nameLength);

                    // Read type
                    duk_get_prop(c, -3);

                    size_t callbackLength;
                    const char* callbackStr = duk_to_lstring(c, -1, &callbackLength);
                    std::string callback = std::string(callbackStr, callbackLength);

                    // Add event
                    Ref<Event> event = boost::make_shared<JSEvent>(
                        boost::dynamic_pointer_cast<JSScript>(shared_from_this()), callback);
                    if (event != nullptr)
                    {
                        // Insert property
                        eventList[name] = event;
                    }

                    // Pop type
                    duk_pop(c);
                }

                // Pop enum
                duk_pop(c);
            }
            void JSScript::InitializeTimers()
            {
                assert(context != nullptr);

                duk_context* c = context.get();

                // Get timers object
                duk_get_global_lstring(c, "timers", 6);

                // Iterate over every property in 'timers'
                duk_enum(c, -1, 0);
                while (duk_next(c, -1, 0))
                {
                    size_t nameLength;
                    const char* nameStr = duk_to_lstring(c, -1, &nameLength);
                    std::string name = std::string(nameStr, nameLength);

                    // Read type
                    duk_get_prop(c, -3);

                    size_t callbackLength;
                    const char* callbackStr = duk_to_lstring(c, -1, &callbackLength);
                    std::string callback = std::string(callbackStr, callbackLength);

                    // Add timer
                    Ref<Timer> timer = boost::make_shared<JSTimer>(
                        boost::dynamic_pointer_cast<JSScript>(shared_from_this()), callback);
                    if (timer != nullptr)
                    {
                        // Insert property
                        eventList[name] = timer;
                    }

                    // Pop type
                    duk_pop(c);
                }

                // Pop enum
                duk_pop(c);
            }

            bool JSScript::Initialize()
            {
                // Lock main mutex
                boost::lock_guard lock(mutex);

                // Check if compilation stage is needed
                size_t c = scriptSource->GetChecksum();
                if (c != checksum || context == nullptr)
                {
                    context = Ref<duk_context>(duk_create_heap(nullptr, nullptr, nullptr, this, nullptr),
                                               [](duk_context* context) -> void { duk_destroy_heap(context); });
                    if (context == nullptr)
                    {
                        checksum = 0;
                        return false;
                    }

                    // Prepare context
                    JSTuple tuple = {boost::dynamic_pointer_cast<JSScript>(shared_from_this())};
                    if (duk_safe_call(context.get(), JSScript::InitializeSafe, (void*)&tuple, 0, 1) != DUK_EXEC_SUCCESS)
                    {
                        // TODO Error message duk_safe_to_string(context, -1);
                        LOG_WARNING("Duktape: {0}", std::string(duk_safe_to_string(context.get(), -1)));

                        context = nullptr;
                        checksum = 0;

                        return false;
                    }
                    else
                        duk_pop(context.get());

                    // Set checksum to prevent recompilation
                    checksum = c;
                }

                return true;
            }

            struct JSInvokeTuple
            {
                std::string event;
            };

            duk_ret_t JSScript::InvokeSafe(duk_context* context, void* udata)
            {
                JSScript* script = (JSScript*)duk_get_user_data(context);

                auto& [event] = *(JSInvokeTuple*)udata;

                // Get events object
                if (!duk_get_global_lstring(context, event.data(), event.size()))
                    return DUK_RET_ERROR;

                // Prepare timout
                script->PrepareTimeout(5000); // 5 seconds

                // Execute event
                duk_call(context, 0);
                duk_pop(context);

                return DUK_EXEC_SUCCESS;
            }

            duk_ret_t JSScript::PropertyGetter(duk_context* context)
            {
                JSScript* script = (JSScript*)duk_get_user_data(context);
                uint32_t index = duk_get_current_magic(context);

                if (index < script->propertyByIDList.size())
                {
                    Ref<Property> property = script->propertyByIDList[index];
                    assert(property != nullptr);

                    switch (property->GetType())
                    {
                    case PropertyType::kBooleanType:
                        duk_push_boolean(context, property->GetBoolean());
                        return 1;
                    case PropertyType::kIntegerType:
                        duk_push_int(context, property->GetInteger());
                        return 1;
                    case PropertyType::kNumberType:
                        duk_push_number(context, property->GetNumber());
                        return 1;
                    case PropertyType::kStringType: {
                        std::string string = property->GetString();
                        duk_push_lstring(context, string.data(), string.size());
                        return 1;
                    }
                    case PropertyType::kEndpointType: {
                        duk_push_object(context);

                        Endpoint endpoint = property->GetEndpoint();

                        duk_push_lstring(context, endpoint.host.data(), endpoint.host.size());
                        duk_put_prop_lstring(context, -2, "host", 4);

                        duk_push_int(context, endpoint.port);
                        duk_put_prop_lstring(context, -2, "port", 4);

                        return 1;
                    }
                    case PropertyType::kColorType: {
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
            duk_ret_t JSScript::PropertySetter(duk_context* context)
            {
                JSScript* script = (JSScript*)duk_get_user_data(context);
                uint32_t index = duk_get_current_magic(context);

                if (index < script->propertyByIDList.size() && duk_get_top(context) == 1)
                {
                    Ref<Property> property = script->propertyByIDList[index];
                    assert(property != nullptr);

                    switch (property->GetType())
                    {
                    case PropertyType::kBooleanType: {
                        property->SetBoolean(duk_to_boolean(context, -1));
                        duk_pop(context);

                        return 0;
                    }
                    case PropertyType::kIntegerType: {
                        property->SetInteger(duk_to_int(context, -1));
                        duk_pop(context);

                        return 0;
                    }
                    case PropertyType::kNumberType: {
                        property->SetNumber(duk_to_number(context, -1));
                        duk_pop(context);

                        return 0;
                    }
                    case PropertyType::kStringType: {
                        size_t length;
                        const char* string = duk_to_lstring(context, -1, &length);
                        property->SetString(std::string(string, length));
                        duk_pop(context);

                        return 0;
                    }
                    case PropertyType::kEndpointType: {
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
                    case PropertyType::kColorType: {
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

            bool JSScript::InvokeImpl(const std::string& event)
            {
            }
            // bool JSScript::Invoke(Ref<View> sender, const std::string& event)
            // {
            //     if (context != nullptr)
            //     {
            //         JSInvokeTuple tuple = {event};
            //         if (duk_safe_call(context.get(), JSScript::InvokeSafe, (void*)&tuple, 0, 1) != DUK_EXEC_SUCCESS)
            //         {
            //             // TODO Error message duk_safe_to_string(context, -1);
            //             LOG_WARNING("Duktape: {0}", std::string(duk_safe_to_string(context.get(), -1)));

            //             duk_pop(context.get());
            //             return false;
            //         }
            //         else
            //             duk_pop(context.get());

            //         return true;
            //     }
            //     else
            //         return false;
            // }

            bool JSScript::Terminate()
            {
                LOG_CODE_MISSING("Terminate js script.");
                return true;
            }
        }
    }
}