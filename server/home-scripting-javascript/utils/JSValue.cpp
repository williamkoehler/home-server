#include "JSValue.hpp"
#include "../literals.hpp"

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            duk_ret_t duk_endpoint_constructor(duk_context* context);
            duk_ret_t duk_color_constructor(duk_context* context);
            duk_ret_t duk_room_id_constructor(duk_context* context);
            duk_ret_t duk_device_id_constructor(duk_context* context);
            duk_ret_t duk_service_id_constructor(duk_context* context);

            duk_ret_t duk_get_value_type(duk_context* context)
            {
                // Expect [ value ]
                if (duk_get_top(context) != 1)
                    return DUK_RET_ERROR;

                ValueType type = ValueType::kUnknownType;

                switch (duk_get_type(context, -1))
                {
                case DUK_TYPE_NULL:
                    type = ValueType::kNullType;
                    break;
                case DUK_TYPE_BOOLEAN:
                    type = ValueType::kBooleanType;
                    break;
                case DUK_TYPE_NUMBER:
                    type = ValueType::kNumberType;
                    break;
                case DUK_TYPE_STRING:
                    type = ValueType::kStringType;
                    break;
                case DUK_TYPE_OBJECT:
                {
                    // Get type
                    duk_get_prop_lstring(context, -1, TYPE_PROPERTY, TYPE_PROPERTY_SIZE); // [ value string ]

                    size_t typeLength;
                    const char* typeStr = duk_to_lstring(context, -1, &typeLength); // [ value string ]

                    type = ParseValueType(std::string_view(typeStr, typeLength));

                    // Pop type
                    duk_pop(context); // [ value ]
                }
                default:
                    break;
                }

                // Pop value
                duk_pop(context); // [ ]

                std::string_view type2 = StringifyValueTypeConst(type);
                duk_push_lstring(context, type2.data(), type2.size()); // [ string ]

                return 1; // [ string ]
            }

            void duk_new_value(duk_context* context, Ref<Value> value)
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
                    case ValueType::kRoomIDType:
                    {
                        duk_new_room_id(context, value->GetRoomID());
                        break;
                    }
                    case ValueType::kDeviceIDType:
                    {
                        duk_new_device_id(context, value->GetDeviceID());
                        break;
                    }
                    case ValueType::kServiceIDType:
                    {
                        duk_new_service_id(context, value->GetServiceID());
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

            Ref<Value> duk_get_value(duk_context* context, duk_idx_t idx)
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
                    duk_get_prop_lstring(context, idx, TYPE_PROPERTY, TYPE_PROPERTY_SIZE); // [ string ]

                    size_t typeLength;
                    const char* typeStr = duk_get_lstring(context, -1, &typeLength); // [ string ]
                    uint32_t typeHash = crc32(typeStr, typeLength);

                    // Pop number
                    duk_pop(context);

                    switch (typeHash)
                    {
                    case CRC32(ENDPOINT_TYPE_NAME):
                    {
                        // Get host
                        duk_get_prop_lstring(context, idx, HOST_PROPERTY, HOST_PROPERTY_SIZE); // [ string ]

                        size_t hostLength;
                        const char* hostStr = duk_get_lstring(context, -1, &hostLength);
                        std::string host = std::string(hostStr, hostLength); // [ string ]

                        // Get port
                        duk_get_prop_lstring(context, idx, PORT_PROPERTY, PORT_PROPERTY_SIZE); // [ string number ]

                        uint16_t port = (uint16_t)duk_get_uint(context, -1); // [ string number ]

                        duk_pop_2(context); // [ ]

                        return Value::CreateEndpoint(Endpoint{
                            .host = std::move(host),
                            .port = port,
                        });
                    }
                    case CRC32(COLOR_TYPE_NAME):
                    {
                        // Get red, green, and blue
                        duk_get_prop_lstring(context, idx, RED_PROPERTY, RED_PROPERTY_SIZE);     // [ number ]
                        duk_get_prop_lstring(context, idx, GREEN_PROPERTY, GREEN_PROPERTY_SIZE); // [ number number ]
                        duk_get_prop_lstring(context, idx, BLUE_PROPERTY,
                                             BLUE_PROPERTY_SIZE); // [ number number number ]

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
            void duk_get_value(duk_context* context, duk_idx_t idx, Ref<Value> value)
            {
                switch (value->GetType())
                {
                case ValueType::kUnknownType:
                case ValueType::kNullType:
                    if (!duk_is_null_or_undefined(context, idx))
                        duk_error(context, DUK_ERR_TYPE_ERROR, "Expected null or undefined.");
                    break;
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
                        duk_get_prop_lstring(context, idx, "host", 4); // [ string ]

                        size_t hostLength;
                        const char* hostStr = duk_to_lstring(context, -1, &hostLength);
                        std::string host = std::string(hostStr, hostLength); // [ string ]

                        duk_pop(context); // [ ]

                        // Get port
                        duk_get_prop_lstring(context, idx, "port", 4); // [ number ]

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
                case ValueType::kRoomIDType:
                {
                    if (duk_is_object(context, idx))
                    {
                        // Get id
                        duk_get_prop_lstring(context, idx, ID_PROPERTY, ID_PROPERTY_SIZE); // [ number ]
                        uint8_t id = (uint8_t)duk_to_uint(context, -1);                    // [ number ]
                        duk_pop(context);                                                  // [ ]

                        value->SetRoomID(id);
                    }
                    else
                        duk_error(context, DUK_ERR_TYPE_ERROR, "Expected object.");
                    break;
                }
                case ValueType::kDeviceIDType:
                {
                    if (duk_is_object(context, idx))
                    {
                        // Get id
                        duk_get_prop_lstring(context, idx, ID_PROPERTY, ID_PROPERTY_SIZE); // [ number ]
                        uint8_t id = (uint8_t)duk_to_uint(context, -1);                    // [ number ]
                        duk_pop(context);                                                  // [ ]

                        value->SetDeviceID(id);
                    }
                    else
                        duk_error(context, DUK_ERR_TYPE_ERROR, "Expected object.");
                    break;
                }
                case ValueType::kServiceIDType:
                {
                    if (duk_is_object(context, idx))
                    {
                        // Get id
                        duk_get_prop_lstring(context, idx, ID_PROPERTY, ID_PROPERTY_SIZE); // [ number ]
                        uint8_t id = (uint8_t)duk_to_uint(context, -1);                    // [ number ]
                        duk_pop(context);                                                  // [ ]

                        value->SetServiceID(id);
                    }
                    else
                        duk_error(context, DUK_ERR_TYPE_ERROR, "Expected object.");
                    break;
                }
                }
            }

            bool duk_import_value(duk_context* context)
            {
                assert(context != nullptr);

                duk_push_global_object(context); // [ global ]

                // Import endpoint
                {
                    // Push ctor
                    duk_push_c_function(context, duk_endpoint_constructor, DUK_VARARGS); // [ global c_func ]

                    // Register prototype
                    duk_push_object(context);                          // [ global c_func object ]
                    duk_put_prop_lstring(context, -2, "prototype", 9); // [ global c_func ]

                    // Put prop
                    duk_put_prop_lstring(context, -2, ENDPOINT_OBJECT, ENDPOINT_OBJECT_SIZE); // [ global ]
                }

                // Import color
                {
                    // Push ctor
                    duk_push_c_function(context, duk_color_constructor, DUK_VARARGS); // [ global c_func ]

                    // Register prototype
                    duk_push_object(context);                          // [ global c_func object ]
                    duk_put_prop_lstring(context, -2, "prototype", 9); // [ global c_func ]

                    // Put prop
                   duk_put_prop_lstring(context, -2,COLOR_OBJECT, COLOR_OBJECT_SIZE); // [ global c_func string ]
                }

                // Import room id
                {
                    // Push ctor
                    duk_push_c_function(context, duk_room_id_constructor, DUK_VARARGS); // [ global c_func ]

                    // Register prototype
                    duk_push_object(context);                          // [ global c_func object ]
                    duk_put_prop_lstring(context, -2, "prototype", 9); // [ global c_func ]

                    // Put prop
                    duk_put_prop_lstring(context, -2, ROOM_ID_OBJECT, ROOM_ID_OBJECT_SIZE); // [ global ]
                }

                // Import device id
                {
                    // Push ctor
                    duk_push_c_function(context, duk_device_id_constructor, DUK_VARARGS); // [ global c_func ]

                    // Register prototype
                    duk_push_object(context);                          // [ global c_func object ]
                    duk_put_prop_lstring(context, -2, "prototype", 9); // [ global c_func ]

                    // Put prop
                   duk_put_prop_lstring(context, -2, DEVICE_ID_OBJECT, DEVICE_ID_OBJECT_SIZE); // [ global ]
                }

                // Import service id
                {
                    // Push ctor
                    duk_push_c_function(context, duk_service_id_constructor, DUK_VARARGS); // [ global c_func ]

                    // Register prototype
                    duk_push_object(context);                          // [ global c_func object ]
                    duk_put_prop_lstring(context, -2, "prototype", 9); // [ global c_func ]

                    // Put prop
                    duk_put_prop_lstring(context, -2, SERVICE_ID_OBJECT,
                                         SERVICE_ID_OBJECT_SIZE); // [ global ]
                }

                // Register methods
                static const duk_function_list_entry methods[] = {
                    {"getValueType", duk_get_value_type, 1},
                    {nullptr, nullptr, 0},
                };

                duk_put_function_list(context, -1, methods); // [ global ]

                // Pop global
                duk_pop(context);

                return true;
            }
        }
    }
}