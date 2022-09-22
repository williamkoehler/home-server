#include "JSDevice.hpp"
#include <home-scripting/main/DeviceView.hpp>
#include <home-scripting/main/HomeView.hpp>

#include "../utils/JSValue.hpp"

#define DEVICE_OBJECT ("Device")
#define DEVICE_ID_PROPERTY DUK_HIDDEN_SYMBOL("device_id")

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            const char* DeviceObject = DEVICE_OBJECT;
            const size_t DeviceObjectSize = std::size(DEVICE_OBJECT) - 1;

            const char* DeviceIDProperty = DEVICE_ID_PROPERTY;
            const size_t DeviceIDPropertySize = std::size(DEVICE_ID_PROPERTY) - 1;

            bool JSDevice::duk_import(duk_context* context)
            {
                assert(context != nullptr);

                duk_push_c_function(context, JSDevice::duk_constructor, 1); // [ c_func ]
                duk_push_object(context);                                   // [ c_func object ]

                // Register methods
                static const duk_function_list_entry methods[] = {
                    {"isValid", JSDevice::duk_is_valid, 0},
                    {"getName", JSDevice::duk_get_name, 0},
                    {"setName", JSDevice::duk_set_name, 1},
                    {"invoke", JSDevice::duk_invoke, 2},
                    {nullptr, nullptr, 0},
                };

                duk_put_function_list(context, -1, methods); // [ c_func object ]

                // Register prototype and constructor
                duk_put_prop_lstring(context, -2, "prototype", 9);
                duk_put_global_string(context, DEVICE_OBJECT);

                return true;
            }

            duk_ret_t JSDevice::duk_constructor(duk_context* context)
            {
                if (!duk_is_constructor_call(context))
                    return DUK_RET_ERROR;

                // Expect [ number ]
                if (duk_get_top(context) != 1 || !duk_is_number(context, -1))
                    return DUK_RET_ERROR;

                // Push this
                duk_push_this(context); // [ number this ]

                // Set unique id
                duk_dup(context, -2);                                                      // [ number this number ]
                duk_put_prop_lstring(context, -2, DeviceIDProperty, DeviceIDPropertySize); // [ number this ]

                // Pop this and number
                duk_pop_2(context); // [ ]

                return 0;
            }

            duk_ret_t JSDevice::duk_is_valid(duk_context* context)
            {
                // Expect [ ]
                if (duk_get_top(context) == 0)
                {
                    // Get id
                    duk_push_this(context);                                                    // [ this ]
                    duk_get_prop_lstring(context, -1, DeviceIDProperty, DeviceIDPropertySize); // [ this number ]
                    identifier_t deviceID = (identifier_t)duk_get_uint(context, -1);

                    // Pop 2
                    duk_pop_2(context); // [ ]

                    Ref<HomeView> homeView = HomeView::GetHomeView();
                    assert(homeView != nullptr);

                    // Get device view
                    Ref<DeviceView> deviceView = homeView->GetDevice(deviceID);

                    // Push result
                    duk_push_boolean(context, deviceView != nullptr);

                    return 1; // [ boolean ]
                }
                else
                {
                    // Errors
                    return DUK_RET_ERROR;
                }
            }

            duk_ret_t JSDevice::duk_get_name(duk_context* context)
            {
                // Expect [ ]
                if (duk_get_top(context) == 0)
                {
                    // Get id
                    duk_push_this(context);                                                    // [this]
                    duk_get_prop_lstring(context, -1, DeviceIDProperty, DeviceIDPropertySize); // [this number]
                    identifier_t deviceID = (identifier_t)duk_get_uint(context, -1);

                    // Pop 2
                    duk_pop_2(context); // [ ]

                    // Get home view
                    Ref<HomeView> homeView = HomeView::GetHomeView();
                    assert(homeView != nullptr);

                    // Get device view
                    Ref<DeviceView> deviceView = homeView->GetDevice(deviceID);
                    if (deviceView != nullptr)
                    {
                        std::string name = deviceView->GetName();

                        // Push name
                        duk_push_lstring(context, name.data(), name.size()); // [ string ]

                        return 1; // [ string ]
                    }
                    else
                    {
                        // Push undefined
                        duk_push_undefined(context); // [ undefined ]

                        return 1; // [ undefined ]
                    }
                }
                else
                {
                    // Error
                    return DUK_RET_ERROR;
                }
            }
            duk_ret_t JSDevice::duk_set_name(duk_context* context)
            {
                // Expect [ string ]
                if (duk_get_top(context) == 1 && duk_is_string(context, -1))
                {
                    // Get value
                    size_t nameLength;
                    const char* nameStr = duk_get_lstring(context, -1, &nameLength);
                    std::string name = std::string(nameStr, nameLength);

                    // Get id
                    duk_push_this(context);                                                    // [ string this ]
                    duk_get_prop_lstring(context, -1, DeviceIDProperty, DeviceIDPropertySize); // [ string this number ]
                    identifier_t deviceID = (identifier_t)duk_get_uint(context, -1);

                    // Pop this, number and string
                    duk_pop_3(context); // [ string ]

                    // Get home view
                    Ref<HomeView> homeView = HomeView::GetHomeView();
                    assert(homeView != nullptr);

                    // Get device view
                    Ref<DeviceView> deviceView = homeView->GetDevice(deviceID);
                    if (deviceView != nullptr)
                    {
                        // Set name
                        deviceView->SetName(name);

                        return 1; // [ string ]
                    }
                    else
                    {
                        // Push undefined
                        duk_push_undefined(context); // [ undefined ]

                        return 1; // [ undefined ]
                    }
                }
                else
                {
                    // Error
                    return DUK_RET_ERROR;
                }
            }

            duk_ret_t JSDevice::duk_invoke(duk_context* context)
            {
                // Expect [ string value ]
                if (duk_get_top(context) == 2 && duk_is_string(context, -2))
                {
                    // Get name
                    size_t nameLength;
                    const char* nameStr = duk_get_lstring(context, -2, &nameLength);
                    std::string name = std::string(nameStr, nameLength);

                    // Get value
                    Ref<Value> value = JSValue::duk_get_value(context, -1);
                    assert(value != nullptr);

                    // Get id
                    duk_push_this(context); // [ string value this ]
                    duk_get_prop_lstring(context, -1, DeviceIDProperty,
                                         DeviceIDPropertySize); // [ string value this number ]
                    identifier_t deviceID = (identifier_t)duk_get_uint(context, -1);

                    // Pop number, this, value, and string
                    duk_pop_n(context, 4);

                    // Get home view
                    Ref<HomeView> homeView = HomeView::GetHomeView();
                    assert(homeView != nullptr);

                    // Get device view
                    Ref<DeviceView> deviceView = homeView->GetDevice(deviceID);
                    if (deviceView != nullptr)
                    {
                        // Set name
                        deviceView->Invoke(name, value);

                        return 0;
                    }
                    else
                    {
                        // Push undefined
                        duk_push_undefined(context); // [ undefined ]

                        return 1; // [ undefined ]
                    }
                }
                else
                {
                    // Error
                    return DUK_RET_ERROR;
                }
            }

            bool JSDevice::duk_new_device(duk_context* context, Ref<DeviceView> deviceView)
            {
                assert(context != nullptr);
                assert(deviceView != nullptr);

                // New device object
                duk_get_global_lstring(context, DeviceObject, DeviceObjectSize); // [function]
                duk_push_uint(context, deviceView->GetID());                     // [function number]
                duk_new(context, 1);                                             // [object]

                return true;
            }
        }
    }
}