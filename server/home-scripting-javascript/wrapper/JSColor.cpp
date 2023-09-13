#include "JSValue.hpp"
#include "literals.hpp"

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            duk_ret_t duk_color_constructor(duk_context* context)
            {
                if (!duk_is_constructor_call(context))
                    return DUK_RET_ERROR;

                // Expect [ number number number ] ([ red green blue ])
                duk_idx_t argc = duk_get_top(context);
                if (argc != 3 || !duk_is_number(context, 0) || !duk_is_number(context, 1) || !duk_is_number(context, 2))
                {
                    duk_pop_n(context, argc);
                    duk_push_uint(context, 0);
                    duk_push_uint(context, 0);
                    duk_push_uint(context, 0);
                }

                // Push this
                duk_push_this(context); // [ number number number this ]
                duk_insert(context, 0); // [ this number number number ]

                // Set type
                duk_push_lstring(context, COLOR_TYPE_NAME, COLOR_TYPE_NAME_SIZE); // [ this number number number string ]
                duk_put_prop_lstring(context, -5, TYPE_PROPERTY, TYPE_PROPERTY_SIZE); // [ this number number number ]

                duk_put_prop_lstring(context, -4, BLUE_PROPERTY, BLUE_PROPERTY_SIZE);   // [ this number number ]
                duk_put_prop_lstring(context, -3, GREEN_PROPERTY, GREEN_PROPERTY_SIZE); // [ this number ]
                duk_put_prop_lstring(context, -2, RED_PROPERTY, RED_PROPERTY_SIZE);     // [ this ]

                // Seal
                duk_seal(context, -1);

                // Pop this
                duk_pop(context); // [ ]

                return 0;
            }

            bool duk_new_color(duk_context* context, const Color& color)
            {
                assert(context != nullptr);

                // New color object
                duk_get_global_string(context, COLOR_OBJECT); // [ func ]
                duk_push_uint(context, color.red);             // [ func number ]
                duk_push_uint(context, color.green);           // [ func number number ]
                duk_push_uint(context, color.blue);            // [ func number number number ]
                duk_new(context, 3);                          // [ object ]

                return true;
            }
        }
    }
}