#include "JSValue.hpp"
#include "literals.hpp"

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            duk_ret_t duk_device_id_constructor(duk_context* context)
            {
                if (!duk_is_constructor_call(context))
                    return DUK_RET_ERROR;

                // Expect [ number ] ([ id ])
                duk_idx_t argc = duk_get_top(context);
                if (argc != 1 || !duk_is_number(context, 0))
                {
                    duk_pop_n(context, argc);
                    duk_push_uint(context, 0);
                }

                // Push this
                duk_push_this(context); // [ number this ]
                duk_insert(context, 0); // [ this number ]

                // Set type
                duk_push_lstring(context, DEVICE_TYPE_NAME, DEVICE_TYPE_NAME_SIZE);     // [ this number string ]
                duk_put_prop_lstring(context, -3, TYPE_PROPERTY, TYPE_PROPERTY_SIZE); // [ this number ]

                duk_put_prop_lstring(context, -2, ID_PROPERTY, ID_PROPERTY_SIZE); // [ this string ]

                // Seal
                duk_seal(context, -1);

                // Pop this
                duk_pop(context); // [ ]

                return 0;
            }

            bool duk_new_device_id(duk_context* context, identifier_t id)
            {
                assert(context != nullptr);

                // New device id object
                duk_get_global_lstring(context, DEVICE_ID_OBJECT, DEVICE_ID_OBJECT_SIZE); // [ func ]
                duk_push_uint(context, id);                        // [ func number ]
                duk_new(context, 1);                              // [ object ]

                return true;
            }
        }
    }
}