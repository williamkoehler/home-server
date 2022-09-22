#include "JSValue.hpp"

#define CLASS_PROPERTY DUK_HIDDEN_SYMBOL("class")
#define COLOR_OBJECT ("Color")
#define ENDPOINT_OBJECT ("Endpoint")

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            const char* ClassProperty = CLASS_PROPERTY;
            const size_t ClassPropertySize = std::size(CLASS_PROPERTY) - 1;

            const char* EndpointObject = ENDPOINT_OBJECT;
            const size_t EndpointObjectSize = std::size(ENDPOINT_OBJECT) - 1;

            const char* ColorObject = COLOR_OBJECT;
            const size_t ColorObjectSize = std::size(COLOR_OBJECT) - 1;

            bool JSEndpoint::duk_import(duk_context* context)
            {
                assert(context != nullptr);

                duk_push_c_function(context, JSEndpoint::duk_constructor, 2); // [ c_func ]
                duk_push_object(context);                                     // [ c_func object ]

                // Register prototype and constructor
                duk_put_prop_lstring(context, -2, "prototype", 9); // [ c_func ]
                duk_put_global_string(context, ENDPOINT_OBJECT);   // [ ]

                return true;
            }
            duk_ret_t JSEndpoint::duk_constructor(duk_context* context)
            {
                if (!duk_is_constructor_call(context))
                    return DUK_RET_ERROR;

                // Expect [ string number ]
                if (duk_get_top(context) != 2 || !duk_is_string(context, -2) || !duk_is_number(context, -1))
                    return DUK_RET_ERROR;

                // Push this
                duk_push_this(context); // [ string number this ]

                // Set class
                duk_push_lstring(context, "endpoint", 8); // [ string number  this string ]
                duk_put_prop_lstring(context, -2, ClassProperty, ClassPropertySize); // [ string number this ]

                // Set host
                duk_dup(context, -3);                         // [ string number this string ]
                duk_put_prop_lstring(context, -2, "host", 4); // [ string number this ]

                // Set port
                duk_dup(context, -2);                         // [ string number this number ]
                duk_put_prop_lstring(context, -2, "port", 4); // [ string number this ]

                // Seal
                duk_seal(context, -1);

                // Pop this, number and string
                duk_pop_3(context); // [ ]

                return 0;
            }
            bool JSEndpoint::duk_new_endpoint(duk_context* context, const Endpoint& endpoint)
            {
                assert(context != nullptr);

                // New endpoint object
                duk_get_global_string(context, COLOR_OBJECT);                          // [ func ]
                duk_push_lstring(context, endpoint.host.data(), endpoint.host.size()); // [ func string ]
                duk_push_uint(context, endpoint.port);                                 // [ func string number ]
                duk_new(context, 2);                                                   // [ object]

                return true;
            }

            bool JSColor::duk_import(duk_context* context)
            {
                assert(context != nullptr);

                duk_push_c_function(context, JSColor::duk_constructor, 3); // [ c_func ]
                duk_push_object(context);                                  // [ c_func object ]

                // Register prototype and constructor
                duk_put_prop_lstring(context, -2, "prototype", 9); // [ c_func ]
                duk_put_global_string(context, COLOR_OBJECT);      // [ ]

                return true;
            }
            duk_ret_t JSColor::duk_constructor(duk_context* context)
            {
                if (!duk_is_constructor_call(context))
                    return DUK_RET_ERROR;

                // Expect [ number number number ]
                if (duk_get_top(context) != 3 || !duk_is_number(context, -1) || !duk_is_number(context, -2) ||
                    !duk_is_number(context, -3))
                    return DUK_RET_ERROR;

                // Push this
                duk_push_this(context); // [ number number number this ]

                // Set class
                duk_push_lstring(context, "color", 5); // [ number number number this string ]
                duk_put_prop_lstring(context, -2, ClassProperty, ClassPropertySize); // [ number number number this ]

                // Set red
                duk_dup(context, -4);                        // [ number number number this number ]
                duk_put_prop_lstring(context, -2, "red", 3); // [ number number number this ]

                // Set green
                duk_dup(context, -3);                          // [ number number number this number ]
                duk_put_prop_lstring(context, -2, "green", 5); // [ number number number this ]

                // Set blue
                duk_dup(context, -2);                         // [ number number number this number ]
                duk_put_prop_lstring(context, -2, "blue", 4); // [ number number number this ]

                // Seal
                duk_seal(context, -1);

                // Pop this, number, number and number
                duk_pop_n(context, 4); // [ ]

                return 0;
            }
            bool JSColor::duk_new_color(duk_context* context, const Color& color)
            {
                assert(context != nullptr);

                // New color object
                duk_get_global_string(context, COLOR_OBJECT); // [ func ]
                duk_push_uint(context, color.red);            // [ func number ]
                duk_push_uint(context, color.green);          // [ func number ]
                duk_push_uint(context, color.blue);           // [ func number ]
                duk_new(context, 3);                          // [ object ]

                return true;
            }

            void JSValue::duk_push_value(duk_context* context, Ref<Value> value)
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
                        JSEndpoint::duk_new_endpoint(context, endpoint);
                        break;
                    }
                    case ValueType::kColorType:
                    {
                        const Color& color = value->GetColor();
                        JSColor::duk_new_color(context, color);
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
                    // Get class
                    duk_get_prop_lstring(context, idx, ClassProperty, ClassPropertySize); // [ string ]

                    size_t klassLength;
                    const char* klass = duk_to_lstring(context, -1, &klassLength); // [ string ]
                    uint32_t klassHash = crc32(klass, klassLength);

                    // Pop number
                    duk_pop(context);

                    switch (klassHash)
                    {
                    case CRC32("endpoint"):
                    {
                        // Get host
                        duk_get_prop_lstring(context, idx, "host", 4); // [ string ]

                        size_t hostLength;
                        const char* hostStr = duk_to_lstring(context, -1, &hostLength);
                        std::string host = std::string(hostStr, hostLength); // [ string ]

                        duk_pop(context); // [ ]

                        // Get port
                        duk_get_prop_lstring(context, idx, "port", 4); // [ number ]

                        uint16_t port = duk_to_uint16(context, -1); // [ number ]

                        duk_pop(context); // [ ]

                        return Value::CreateEndpoint(Endpoint{
                            .host = std::move(host),
                            .port = port,
                        });
                    }
                    case CRC32("color"):
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
                case ValueType::kNumberType:
                    if (duk_is_number(context, idx))
                        value->SetNumber(duk_get_number(context, idx));
                    else
                        duk_error(context, DUK_ERR_TYPE_ERROR, "Expected number.");
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
                }
                default:
                    if (!duk_is_null_or_undefined(context, idx))
                        duk_error(context, DUK_ERR_TYPE_ERROR, "Expected null or undefined.");
                }
            }
        }
    }
}