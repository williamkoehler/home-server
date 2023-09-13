#include "JSUtils.hpp"
#include "JSScript.hpp"

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
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

            duk_ret_t duk_create_timer(duk_context* context)
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
                    // script->AddTimerTask(method, interval);

                    return 0;
                }
                else
                {
                    // Error
                    return DUK_RET_ERROR;
                }
            }

            duk_ret_t duk_create_delay(duk_context* context)
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

                    // Pop number, and string
                    duk_pop_2(context);

                    Ref<Worker> worker = Worker::GetInstance();
                    assert(worker != nullptr);

                    // Create timer task
                    // script->AddTimerTask(method, interval);

                    return 0;
                }
                else
                {
                    // Error
                    return DUK_RET_ERROR;
                }
            }

            bool duk_import_utils(duk_context* context)
            {
                assert(context != nullptr);

                // Push global object
                duk_push_global_object(context); // [ object ]

                // Register methods
                static const duk_function_list_entry methods[] = {
                    duk_function_list_entry{
                        .key = "print",
                        .value = duk_print,
                        .nargs = DUK_VARARGS,
                    },
                    duk_function_list_entry{
                        .key = "createTimer",
                        .value = duk_create_timer,
                        .nargs = 2,
                    },
                    duk_function_list_entry{
                        .key = "createDelay",
                        .value = duk_create_delay,
                        .nargs = 2,
                    },
                    {nullptr, nullptr, 0},
                };

                duk_put_function_list(context, -1, methods); // [ object ]

                // Pop global object
                duk_pop(context); // [ ]

                return true;
            }
        }
    }
}