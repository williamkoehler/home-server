#include "JSHome.hpp"

#include "../literals.hpp"

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            duk_ret_t duk_device_constructor(duk_context* context);
            duk_ret_t duk_device_is_valid(duk_context* context);
            duk_ret_t duk_device_get_name(duk_context* context);
            duk_ret_t duk_device_set_name(duk_context* context);
            duk_ret_t duk_device_invoke(duk_context* context);

            duk_ret_t duk_room_constructor(duk_context* context);
            duk_ret_t duk_room_is_valid(duk_context* context);
            duk_ret_t duk_room_get_name(duk_context* context);
            duk_ret_t duk_room_set_name(duk_context* context);

            bool duk_import_home(duk_context* context)
            {
                assert(context != nullptr);

                duk_push_global_object(context); // [ global ]

                // Import home
                {
                    // // Push ctor
                    // duk_push_c_function(context, duk_endpoint_constructor, DUK_VARARGS); // [ global c_func ]

                    // // Register prototype
                    // duk_push_object(context);                          // [ global c_func object ]
                    // duk_put_prop_lstring(context, -2, "prototype", 9); // [ global c_func ]

                    // // Put prop
                    // duk_push_lstring(context, ENDPOINT_OBJECT, ENDPOINT_OBJECT_SIZE); // [ global c_func string ]
                    // duk_def_prop(context, -3,
                    //              DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_CLEAR_WEC |
                    //                  DUK_DEFPROP_FORCE); // [ global ]
                }

                // Import room
                {
                    // Push ctor
                    duk_push_c_function(context, duk_room_constructor, 1); // [ global c_func ]

                    // Register prototype
                    duk_push_object(context); // [ global c_func object ]

                    // Register methods
                    static const duk_function_list_entry methods[] = {
                        {"isValid", duk_room_is_valid, 0},
                        {"getName", duk_room_get_name, 0},
                        {"setName", duk_room_set_name, 1},
                        {nullptr, nullptr, 0},
                    };

                    duk_put_function_list(context, -1, methods); // [ c_func object ]

                    duk_put_prop_lstring(context, -2, "prototype", 9); // [ global c_func ]

                    // Put prop
                    duk_put_prop_lstring(context, -2, ROOM_OBJECT, ROOM_OBJECT_SIZE); // [ global ]
                }

                // Import device
                {
                    // Push ctor
                    duk_push_c_function(context, duk_device_constructor, 1); // [ global c_func ]

                    // Register prototype
                    duk_push_object(context); // [ global c_func object ]

                    // Register methods
                    static const duk_function_list_entry methods[] = {
                        {"isValid", duk_device_is_valid, 0},
                        {"getName", duk_device_get_name, 0},
                        {"setName", duk_device_set_name, 1},
                        {"invoke", duk_device_invoke, 2},
                        {nullptr, nullptr, 0},
                    };

                    duk_put_function_list(context, -1, methods); // [ c_func object ]

                    duk_put_prop_lstring(context, -2, "prototype", 9); // [ global c_func ]

                    // Put prop
                    duk_put_prop_lstring(context, -2, DEVICE_OBJECT, DEVICE_OBJECT_SIZE); // [ global ]
                }

                // Pop global
                duk_pop(context);

                return true;
            }
        }
    }
}