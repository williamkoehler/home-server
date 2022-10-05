#include "JSValue.hpp"

#define VALUE_OBJECT ("Value")

#define COLOR_OBJECT ("Color")
#define ENDPOINT_OBJECT ("Endpoint")

#define TYPE_PROPERTY DUK_HIDDEN_SYMBOL("type")

#define VALUE_PROPERTY DUK_HIDDEN_SYMBOL("v")

#define HOST_PROPERTY ("host")
#define PORT_PROPERTY ("port")

#define RED_PROPERTY ("red")
#define GREEN_PROPERTY ("green")
#define BLUE_PROPERTY ("blue")

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            const char* EndpointObject = ENDPOINT_OBJECT;
            const size_t EndpointObjectSize = std::size(ENDPOINT_OBJECT) - 1;

            const char* ColorObject = COLOR_OBJECT;
            const size_t ColorObjectSize = std::size(COLOR_OBJECT) - 1;

            const char* TypeProperty = TYPE_PROPERTY;
            const size_t TypePropertySize = std::size(TYPE_PROPERTY) - 1;

            const char* HostProperty = HOST_PROPERTY;
            const size_t HostPropertySize = std::size(HOST_PROPERTY) - 1;

            const char* PortProperty = PORT_PROPERTY;
            const size_t PortPropertySize = std::size(PORT_PROPERTY) - 1;

            const char* RedProperty = RED_PROPERTY;
            const size_t RedPropertySize = std::size(RED_PROPERTY) - 1;

            const char* GreenProperty = GREEN_PROPERTY;
            const size_t GreenPropertySize = std::size(GREEN_PROPERTY) - 1;

            const char* BlueProperty = BLUE_PROPERTY;
            const size_t BluePropertySize = std::size(BLUE_PROPERTY) - 1;

            duk_ret_t JSValue::duk_endpoint_constructor(duk_context* context)
            {
                if (!duk_is_constructor_call(context))
                    return DUK_RET_ERROR;

                // Expect [ number number number ] ([ red green blue ])
                duk_idx_t argc = duk_get_top(context);
                if (argc != 2 || !duk_is_string(context, 0) || !duk_is_number(context, 1))
                {
                    duk_pop_n(context, argc);
                    duk_push_lstring(context, "", 0);
                    duk_push_uint(context, 0);
                }

                // Push this
                duk_push_this(context); // [ number number number this ]
                duk_insert(context, 0); // [ this number number number ]

                // Set class
                duk_push_lstring(context, "endpoint", 8);                          // [ this string number string ]
                duk_put_prop_lstring(context, -5, TypeProperty, TypePropertySize); // [ this string number ]

                duk_put_prop_lstring(context, 0, "port", 4); // [ this string ]
                duk_put_prop_lstring(context, 0, "host", 4); // [ this ]

                // Seal
                duk_seal(context, -1);

                // Pop this
                duk_pop(context); // [ ]

                return 0;
            }
            bool JSValue::duk_new_endpoint(duk_context* context, const Endpoint& endpoint)
            {
                assert(context != nullptr);

                // New endpoint object
                duk_get_global_string(context, COLOR_OBJECT);                          // [ func ]
                duk_push_lstring(context, endpoint.host.data(), endpoint.host.size()); // [ func string ]
                duk_push_uint(context, endpoint.port);                                 // [ func string number ]
                duk_new(context, 2);                                                   // [ object]

                return true;
            }

            duk_ret_t JSValue::duk_color_constructor(duk_context* context)
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

                // Set class
                duk_push_lstring(context, "color", 5); // [ this number number number string ]
                duk_put_prop_lstring(context, -5, TypeProperty, TypePropertySize); // [ this number number number ]

                duk_put_prop_lstring(context, 0, "blue", 4);  // [ this number number ]
                duk_put_prop_lstring(context, 0, "green", 5); // [ this number ]
                duk_put_prop_lstring(context, 0, "red", 3);   // [ this ]

                // Seal
                duk_seal(context, -1);

                // Pop this
                duk_pop(context); // [ ]

                return 0;
            }
            bool JSValue::duk_new_color(duk_context* context, const Color& color)
            {
                assert(context != nullptr);

                // New color object
                duk_get_global_string(context, COLOR_OBJECT); // [ func ]
                duk_get_uint(context, color.red);             // [ func number ]
                duk_get_uint(context, color.green);           // [ func number number ]
                duk_get_uint(context, color.blue);            // [ func number number number ]
                duk_new(context, 3);                          // [ object ]

                return true;
            }

            duk_ret_t JSValue::duk_get_value_type(duk_context* context)
            {
                // Expect [ value ]
                if (duk_get_top(context) != 0)
                    return DUK_RET_ERROR;

                ValueType type = ValueType::kUnknownType;

                // Push this
                duk_push_this(context); // [ this ]

                // Get type
                duk_get_prop_lstring(context, -1, TypeProperty, TypePropertySize); // [ this string ]

                size_t typeLength;
                const char* typeStr = duk_to_lstring(context, -1, &typeLength); // [ string ]

                type = ParseValueType(std::string_view(typeStr, typeLength));

                // Pop type, and this
                duk_pop_2(context);

                std::string_view type2 = StringifyValueTypeConst(type);
                duk_push_lstring(context, type2.data(), type2.size()); // [ string ]

                return 1; // [ string ]
            }

            bool JSValue::duk_import(duk_context* context)
            {
                assert(context != nullptr);

                duk_push_global_object(context); // [ global ]

                // Import endpoint
                {
                    // Push ctor
                    duk_push_c_function(context, JSValue::duk_endpoint_constructor, DUK_VARARGS); // [ global c_func ]

                    // Register prototype
                    duk_push_object(context);                          // [ global c_func object ]
                    duk_put_prop_lstring(context, -2, "prototype", 9); // [ global c_func ]

                    // Put prop
                    duk_push_lstring(context, EndpointObject, EndpointObjectSize); // [ global c_func string ]
                    duk_def_prop(context, -3,
                                 DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_CLEAR_WEC |
                                     DUK_DEFPROP_FORCE); // [ global ]
                }

                // Import color
                {
                    // Push ctor
                    duk_push_c_function(context, JSValue::duk_color_constructor, DUK_VARARGS); // [ global c_func ]

                    // Register prototype
                    duk_push_object(context);                          // [ global c_func object ]
                    duk_put_prop_lstring(context, -2, "prototype", 9); // [ global c_func ]

                    // Put prop
                    duk_push_lstring(context, ColorObject, ColorObjectSize); // [ global c_func string ]
                    duk_def_prop(context, -3,
                                 DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_CLEAR_WEC |
                                     DUK_DEFPROP_FORCE); // [ global ]
                }

                // Register methods
                static const duk_function_list_entry methods[] = {
                    {"getValueType", JSValue::duk_get_value_type, 0},
                    {nullptr, nullptr, 0},
                };

                duk_put_function_list(context, -1, methods); // [ global ]

                // Pop global
                duk_pop(context);

                return true;
            }

            void JSValue::duk_new_value(duk_context* context, Ref<Value> value)
            {
                if (value != nullptr)
                {
                    switch (value->GetType())
                    {
                    case ValueType::kBooleanType:
                        duk_push_boolean(context, value->GetBoolean());
                        break;
                    case ValueType::kNumberType:
                        duk_push_number(context, value->GetNumber());
                        break;
                    case ValueType::kStringType:
                    {
                        const std::string& string = value->GetString();
                        duk_push_lstring(context, string.data(), string.size());
                        break;
                    }
                    case ValueType::kEndpointType:
                    {
                        const Endpoint& endpoint = value->GetEndpoint();
                        duk_new_endpoint(context, endpoint);
                        break;
                    }
                    case ValueType::kColorType:
                    {
                        const Color& color = value->GetColor();
                        duk_new_color(context, color);
                        break;
                    }
                    default:
                        duk_push_null(context);
                        break;
                    }
                }
                else
                {
                    duk_push_null(context);
                }
            }

            Ref<Value> JSValue::duk_get_value(duk_context* context, duk_idx_t idx)
            {
                switch (duk_get_type(context, idx))
                {
                case DUK_TYPE_BOOLEAN:
                    return Value::CreateBoolean(duk_get_boolean(context, idx));
                case DUK_TYPE_NUMBER:
                    return Value::CreateNumber(duk_get_number(context, idx));
                case DUK_TYPE_STRING:
                {
                    size_t stringLength;
                    const char* string = duk_get_lstring(context, idx, &stringLength);
                    return Value::CreateStringView(std::string_view(string, stringLength));
                }
                case DUK_TYPE_OBJECT:
                {
                    idx = duk_normalize_index(context, idx);

                    // Get type
                    duk_get_prop_lstring(context, idx, TypeProperty, TypePropertySize); // [ string ]

                    size_t typeLength;
                    const char* typeStr = duk_get_lstring(context, -1, &typeLength); // [ string ]
                    uint32_t typeHash = crc32(typeStr, typeLength);

                    // Pop number
                    duk_pop(context);

                    switch (typeHash)
                    {
                    case CRC32("endpoint"):
                    {
                        // Get host
                        duk_get_prop_lstring(context, idx, "host", 4); // [ string ]

                        size_t hostLength;
                        const char* hostStr = duk_get_lstring(context, -1, &hostLength);
                        std::string host = std::string(hostStr, hostLength); // [ string ]

                        // Get port
                        duk_get_prop_lstring(context, idx, "port", 4); // [ string number ]

                        uint16_t port = (uint16_t)duk_get_uint(context, -1); // [ string number ]

                        duk_pop_2(context); // [ ]

                        return Value::CreateEndpoint(Endpoint{
                            .host = std::move(host),
                            .port = port,
                        });
                    }
                    case CRC32("color"):
                    {
                        // Get red, green, and blue
                        duk_get_prop_lstring(context, idx, "red", 3);   // [ number ]
                        duk_get_prop_lstring(context, idx, "green", 5); // [ number number ]
                        duk_get_prop_lstring(context, idx, "blue", 4);  // [ number number number ]

                        uint8_t red = (uint8_t)duk_get_uint(context, -3);
                        uint8_t green = (uint8_t)duk_get_uint(context, -2);
                        uint8_t blue = (uint8_t)duk_get_uint(context, -1);

                        // Pop blue, green, and blue
                        duk_pop_3(context);

                        return Value::CreateColor(Color{
                            .red = red,
                            .green = green,
                            .blue = blue,
                        });
                    }
                    default:
                        break;
                    }
                }
                default:
                    break;
                }

                return Value::CreateNull();
            }
            void JSValue::duk_get_value(duk_context* context, duk_idx_t idx, Ref<Value> value)
            {
                switch (value->GetType())
                {
                case ValueType::kBooleanType:
                    if (duk_is_boolean(context, idx))
                        value->SetBoolean(duk_get_boolean(context, idx));
                    else
                        duk_error(context, DUK_ERR_TYPE_ERROR, "Expected boolean.");
                    break;
                case ValueType::kNumberType:
                    if (duk_is_number(context, idx))
                        value->SetNumber(duk_get_number(context, idx));
                    else
                        duk_error(context, DUK_ERR_TYPE_ERROR, "Expected number.");
                    break;
                case ValueType::kStringType:
                {
                    if (duk_is_string(context, idx))
                    {
                        size_t stringLength;
                        const char* string = duk_get_lstring(context, idx, &stringLength);
                        value->SetStringView(std::string_view(string, stringLength));
                    }
                    else
                        duk_error(context, DUK_ERR_TYPE_ERROR, "Expected string.");
                    break;
                }
                case ValueType::kEndpointType:
                {
                    if (duk_is_object(context, idx))
                    {
                        // Get host
                        duk_get_prop_lstring(context, idx - 1, "host", 4); // [ string ]

                        size_t hostLength;
                        const char* hostStr = duk_to_lstring(context, -1, &hostLength);
                        std::string host = std::string(hostStr, hostLength); // [ string ]

                        duk_pop(context); // [ ]

                        // Get port
                        duk_get_prop_lstring(context, idx - 2, "port", 4); // [ number ]

                        uint16_t port = duk_to_uint16(context, -1); // [ number ]

                        duk_pop(context); // [ ]

                        value->SetEndpoint(Endpoint{
                            .host = std::move(host),
                            .port = port,
                        });
                    }
                    else
                        duk_error(context, DUK_ERR_TYPE_ERROR, "Expected object.");
                    break;
                }
                case ValueType::kColorType:
                {
                    if (duk_is_object(context, idx))
                    {
                        // Get red
                        duk_get_prop_lstring(context, idx, "red", 3);    // [ number ]
                        uint8_t red = (uint8_t)duk_to_uint(context, -1); // [ number ]
                        duk_pop(context);                                // [ ]

                        // Get green
                        duk_get_prop_lstring(context, idx, "green", 5);    // [ number ]
                        uint8_t green = (uint8_t)duk_to_uint(context, -1); // [ number ]
                        duk_pop(context);                                  // [ ]

                        // Get blue
                        duk_get_prop_lstring(context, idx, "blue", 4);    // [ number ]
                        uint8_t blue = (uint8_t)duk_to_uint(context, -1); // [ number ]
                        duk_pop(context);                                 // [ ]

                        value->SetColor(Color{
                            .red = red,
                            .green = green,
                            .blue = blue,
                        });
                    }
                    else
                        duk_error(context, DUK_ERR_TYPE_ERROR, "Expected object.");
                    break;
                }
                default:
                    if (!duk_is_null_or_undefined(context, idx))
                        duk_error(context, DUK_ERR_TYPE_ERROR, "Expected null or undefined.");
                    break;
                }
            }
        }
    }
}