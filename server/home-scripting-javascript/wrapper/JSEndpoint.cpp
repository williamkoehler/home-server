#include "JSValue.hpp"
#include "literals.hpp"

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            duk_ret_t duk_endpoint_constructor(duk_context* context)
            {
                if (!duk_is_constructor_call(context))
                    return DUK_RET_ERROR;

                // Expect [ string ] ([ host port ])
                duk_idx_t argc = duk_get_top(context);
                if (argc != 2 || !duk_is_string(context, 0) || !duk_is_number(context, 1))
                {
                    duk_pop_n(context, argc);
                    duk_push_lstring(context, "", 0);
                    duk_push_uint(context, 0);
                }

                // Push this
                duk_push_this(context); // [ string number this ]
                duk_insert(context, 0); // [ this string number ]

                // Set type
                duk_push_lstring(context, ENDPOINT_TYPE_NAME, ENDPOINT_TYPE_NAME_SIZE); // [ this string number string ]
                duk_put_prop_lstring(context, -4, TYPE_PROPERTY, TYPE_PROPERTY_SIZE);   // [ this string number ]

                duk_put_prop_lstring(context, -3, PORT_PROPERTY, PORT_PROPERTY_SIZE); // [ this string ]
                duk_put_prop_lstring(context, -2, HOST_PROPERTY, HOST_PROPERTY_SIZE); // [ this ]

                // Seal
                duk_seal(context, -1);

                // Pop this
                duk_pop(context); // [ ]

                return 0;
            }

            bool duk_new_endpoint(duk_context* context, const Endpoint& endpoint)
            {
                assert(context != nullptr);

                // New endpoint object
                duk_get_global_string(context, ENDPOINT_OBJECT);                       // [ func ]
                duk_push_lstring(context, endpoint.host.data(), endpoint.host.size()); // [ func string ]
                duk_push_uint(context, endpoint.port);                                 // [ func string number ]
                duk_new(context, 2);                                                   // [ object]

                return true;
            }
        }
    }
}