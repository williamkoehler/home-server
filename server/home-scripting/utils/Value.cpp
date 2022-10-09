#include "Value.hpp"

#define BOOLEAN_TYPE_NAME "boolean"
#define NUMBER_TYPE_NAME "number"
#define STRING_TYPE_NAME "string"
#define ENDPOINT_TYPE_NAME "endpoint"
#define COLOR_TYPE_NAME "color"
#define ROOM_ID_TYPE_NAME "room"
#define DEVICE_ID_TYPE_NAME "device"
#define SERVICE_ID_TYPE_NAME "service"

namespace server
{
    namespace scripting
    {

        std::string_view StringifyValueTypeConst(ValueType type)
        {
            switch (type)
            {
            case ValueType::kBooleanType:
                return BOOLEAN_TYPE_NAME;
            case ValueType::kNumberType:
                return NUMBER_TYPE_NAME;
            case ValueType::kStringType:
                return STRING_TYPE_NAME;
            case ValueType::kEndpointType:
                return ENDPOINT_TYPE_NAME;
            case ValueType::kColorType:
                return COLOR_TYPE_NAME;
            case ValueType::kRoomIDType:
                return ROOM_ID_TYPE_NAME;
            case ValueType::kDeviceIDType:
                return DEVICE_ID_TYPE_NAME;
            case ValueType::kServiceIDType:
                return SERVICE_ID_TYPE_NAME;
            default:
                return "unknown";
            }
        }
        ValueType ParseValueType(const std::string_view& type)
        {
            switch (crc32(type.data(), type.size()))
            {
            case CRC32(BOOLEAN_TYPE_NAME):
                return ValueType::kBooleanType;
            case CRC32(NUMBER_TYPE_NAME):
                return ValueType::kNumberType;
            case CRC32(STRING_TYPE_NAME):
                return ValueType::kStringType;
            case CRC32(ENDPOINT_TYPE_NAME):
                return ValueType::kEndpointType;
            case CRC32(COLOR_TYPE_NAME):
                return ValueType::kColorType;
            case CRC32(ROOM_ID_TYPE_NAME):
                return ValueType::kRoomIDType;
            case CRC32(DEVICE_ID_TYPE_NAME):
                return ValueType::kDeviceIDType;
            case CRC32(SERVICE_ID_TYPE_NAME):
                return ValueType::kServiceIDType;
            default:
                return ValueType::kNullType;
            }
        }

        Value::Value() : type(ValueType::kNullType)
        {
        }
        Value::Value(bool boolean) : type(ValueType::kBooleanType)
        {
            new ((void*)value) bool(boolean);
        }
        Value::Value(double_t number) : type(ValueType::kNumberType)
        {
            new ((void*)value) double_t(number);
        }
        Value::Value(const std::string& string) : type(ValueType::kStringType)
        {
            new ((void*)value) std::string(string);
        }
        Value::Value(const std::string_view& string) : type(ValueType::kStringType)
        {
            new ((void*)value) std::string(string);
        }
        Value::Value(const Endpoint& endpoint) : type(ValueType::kEndpointType)
        {
            new ((void*)value) Endpoint(endpoint);
        }
        Value::Value(const Color& color) : type(ValueType::kColorType)
        {
            new ((void*)value) Color(color);
        }
        Value::Value(ValueType type) : type(type)
        {
            switch (type)
            {
            case ValueType::kBooleanType:
                new ((void*)value) bool();
                break;
            case ValueType::kNumberType:
                new ((void*)value) double_t();
                break;
            case ValueType::kStringType:
                new ((void*)value) std::string();
                break;
            case ValueType::kEndpointType:
                new ((void*)value) std::string();
                break;
            case ValueType::kColorType:
                new ((void*)value) std::string();
                break;
            case ValueType::kRoomIDType:
            case ValueType::kDeviceIDType:
            case ValueType::kServiceIDType:
                new ((void*)value) identifier_t();
                break;
            default:
                break;
            }
        }
        Value::~Value()
        {
            switch (type)
            {
            case ValueType::kStringType:
                ((std::string*)value)->~basic_string();
                break;
            case ValueType::kEndpointType:
                ((Endpoint*)value)->~Endpoint();
                break;
            case ValueType::kColorType:
                ((Color*)value)->~Color();
                break;
            default: // Do nothing for basic types such as bool, int, etc...
                break;
            }
        }

        Ref<Value> Value::Create(ValueType type)
        {
            return boost::make_shared<Value>(type);
        }
        Ref<Value> Value::Create(rapidjson::Value& input)
        {
            switch (input.GetType())
            {
            case rapidjson::kTrueType:
                return boost::make_shared<Value>(true);
            case rapidjson::kFalseType:
                return boost::make_shared<Value>(false);
            case rapidjson::kNumberType:
                return boost::make_shared<Value>(input.GetDouble());
            case rapidjson::kStringType:
                return boost::make_shared<Value>(std::string_view(input.GetString(), input.GetStringLength()));
            case rapidjson::kObjectType:
            {
                rapidjson::Value::MemberIterator classIt = input.FindMember("_class");
                if (classIt != input.MemberEnd() && classIt->value.IsString())
                {
                    switch (crc32(classIt->value.GetString(), classIt->value.GetStringLength()))
                    {
                    case CRC32(ENDPOINT_TYPE_NAME):
                    {
                        rapidjson::Value::MemberIterator hostIt = input.FindMember("host");
                        rapidjson::Value::MemberIterator portIt = input.FindMember("port");
                        if (hostIt != input.MemberEnd() && hostIt->value.IsString() && portIt != input.MemberEnd() &&
                            portIt->value.IsUint())
                        {
                            Endpoint endpoint = Endpoint{
                                .host = std::string(hostIt->value.GetString(), hostIt->value.GetStringLength()),
                                .port = (uint16_t)portIt->value.GetUint(),
                            };

                            return boost::make_shared<Value>(endpoint);
                        }
                    }
                    case CRC32(COLOR_TYPE_NAME):
                    {
                        rapidjson::Value::MemberIterator rIt = input.FindMember("r");
                        rapidjson::Value::MemberIterator gIt = input.FindMember("g");
                        rapidjson::Value::MemberIterator bIt = input.FindMember("b");
                        if (rIt->value.IsUint() && gIt != input.MemberEnd() && gIt->value.IsUint() &&
                            bIt != input.MemberEnd() && bIt->value.IsUint())
                        {
                            Color color = Color{
                                .red = (uint8_t)rIt->value.GetUint(),
                                .green = (uint8_t)gIt->value.GetUint(),
                                .blue = (uint8_t)bIt->value.GetUint(),
                            };

                            return boost::make_shared<Value>(color);
                        }
                    }
                    case CRC32(ROOM_ID_TYPE_NAME):
                    {
                        rapidjson::Value::MemberIterator idIt = input.FindMember("id");
                        if (idIt != input.MemberEnd() && idIt->value.IsUint())
                        {
                            Ref<Value> value = boost::make_shared<Value>(ValueType::kRoomIDType);
                            value->SetRoomID(idIt->value.GetUint());

                            return value;
                        }
                    }
                    case CRC32(DEVICE_ID_TYPE_NAME):
                    {
                        rapidjson::Value::MemberIterator idIt = input.FindMember("id");
                        if (idIt != input.MemberEnd() && idIt->value.IsUint())
                        {
                            Ref<Value> value = boost::make_shared<Value>(ValueType::kDeviceIDType);
                            value->SetRoomID(idIt->value.GetUint());

                            return value;
                        }
                    }
                    case CRC32(SERVICE_ID_TYPE_NAME):
                    {
                        rapidjson::Value::MemberIterator idIt = input.FindMember("id");
                        if (idIt != input.MemberEnd() && idIt->value.IsUint())
                        {
                            Ref<Value> value = boost::make_shared<Value>(ValueType::kServiceIDType);
                            value->SetRoomID(idIt->value.GetUint());

                            return value;
                        }
                    }
                    }
                }

                break;
            }
            default:
                break;
            }

            return boost::make_shared<Value>();
        }

        rapidjson::Value Value::JsonGet(rapidjson::Document::AllocatorType& allocator)
        {
            switch (type)
            {
            case ValueType::kBooleanType:
                return rapidjson::Value(*(bool*)value);
            case ValueType::kNumberType:
                return rapidjson::Value(*(double_t*)value);
            case ValueType::kStringType:
            {
                const std::string& string = *(std::string*)value;
                return rapidjson::Value(string.data(), string.size(), allocator);
            }
            case ValueType::kEndpointType:
            {
                const Endpoint& endpoint = *(Endpoint*)value;

                rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);

                json.AddMember("_class", rapidjson::Value("endpoint"), allocator);
                json.AddMember("host", rapidjson::Value(endpoint.host.data(), endpoint.host.size(), allocator),
                               allocator);
                json.AddMember("port", rapidjson::Value(endpoint.port), allocator);

                return json;
            }
            case ValueType::kColorType:
            {
                const Color& color = *(Color*)value;

                rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);

                json.AddMember("_class", rapidjson::Value("color"), allocator);
                json.AddMember("r", rapidjson::Value(color.red), allocator);
                json.AddMember("g", rapidjson::Value(color.green), allocator);
                json.AddMember("b", rapidjson::Value(color.blue), allocator);

                return json;
            }
            case ValueType::kRoomIDType:
            {
                const identifier_t& id = *(identifier_t*)value;

                rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);

                json.AddMember("_class", rapidjson::Value("room"), allocator);
                json.AddMember("id", rapidjson::Value(id), allocator);

                return json;
            }
            case ValueType::kDeviceIDType:
            {
                const identifier_t& id = *(identifier_t*)value;

                rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);

                json.AddMember("_class", rapidjson::Value("device"), allocator);
                json.AddMember("id", rapidjson::Value(id), allocator);

                return json;
            }
            case ValueType::kServiceIDType:
            {
                const identifier_t& id = *(identifier_t*)value;

                rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);

                json.AddMember("_class", rapidjson::Value("service"), allocator);
                json.AddMember("id", rapidjson::Value(id), allocator);

                return json;
            }
            default:
                return rapidjson::Value(rapidjson::kNullType);
            }
        }

        bool Value::JsonSet(rapidjson::Value& input)
        {
            switch (type)
            {
            case ValueType::kBooleanType:
                if (input.IsBool())
                    *((bool*)value) = input.GetBool();
                break;
            case ValueType::kNumberType:
                if (input.IsNumber())
                    *((bool*)value) = input.GetDouble();
                break;
            case ValueType::kStringType:
            {
                std::string& string = *(std::string*)value;

                if (input.IsString())
                    string.assign(input.GetString(), input.GetStringLength());
                break;
            }
            case ValueType::kEndpointType:
            {
                Endpoint& endpoint = *(Endpoint*)value;

                if (input.IsObject())
                {
                    rapidjson::Value::MemberIterator classIt = input.FindMember("_class");
                    rapidjson::Value::MemberIterator hostIt = input.FindMember("host");
                    rapidjson::Value::MemberIterator portIt = input.FindMember("port");
                    if (classIt != input.MemberEnd() && classIt->value.IsString() && hostIt != input.MemberEnd() &&
                        hostIt->value.IsString() && portIt != input.MemberEnd() && portIt->value.IsUint())
                    {
                        // Check class
                        if (crc32(classIt->value.GetString(), classIt->value.GetStringLength()) ==
                            CRC32(ENDPOINT_TYPE_NAME))
                        {
                            size_t length = hostIt->value.GetStringLength();
                            endpoint.host = std::string(hostIt->value.GetString(), length);

                            endpoint.port = (uint16_t)portIt->value.GetUint();

                            return true;
                        }
                    }
                }
                break;
            }
            case ValueType::kColorType:
            {
                Color& color = *(Color*)value;

                if (input.IsObject())
                {
                    rapidjson::Value::MemberIterator classIt = input.FindMember("_class");
                    rapidjson::Value::MemberIterator rIt = input.FindMember("r");
                    rapidjson::Value::MemberIterator gIt = input.FindMember("g");
                    rapidjson::Value::MemberIterator bIt = input.FindMember("b");
                    if (classIt != input.MemberEnd() && classIt->value.IsString() && rIt != input.MemberEnd() &&
                        rIt->value.IsUint() && gIt != input.MemberEnd() && gIt->value.IsUint() &&
                        bIt != input.MemberEnd() && bIt->value.IsUint())
                    {
                        // Check class
                        if (crc32(classIt->value.GetString(), classIt->value.GetStringLength()) ==
                            CRC32(COLOR_TYPE_NAME))
                        {
                            color.red = (uint8_t)rIt->value.GetUint();
                            color.green = (uint8_t)gIt->value.GetUint();
                            color.blue = (uint8_t)bIt->value.GetUint();

                            return true;
                        }
                    }
                }
                break;
            }
            case ValueType::kRoomIDType:
            {
                identifier_t& id = *(identifier_t*)value;

                if (input.IsObject())
                {
                    rapidjson::Value::MemberIterator classIt = input.FindMember("_class");
                    rapidjson::Value::MemberIterator idIt = input.FindMember("id");
                    if (classIt != input.MemberEnd() && classIt->value.IsString() && idIt != input.MemberEnd() &&
                        idIt->value.IsUint())
                    {
                        // Check class
                        if (crc32(classIt->value.GetString(), classIt->value.GetStringLength()) ==
                            CRC32(ROOM_ID_TYPE_NAME))
                        {
                            id = (uint8_t)idIt->value.GetUint();

                            return true;
                        }
                    }
                }
                break;
            }
            case ValueType::kDeviceIDType:
            {
                identifier_t& id = *(identifier_t*)value;

                if (input.IsObject())
                {
                    rapidjson::Value::MemberIterator classIt = input.FindMember("_class");
                    rapidjson::Value::MemberIterator idIt = input.FindMember("id");
                    if (classIt != input.MemberEnd() && classIt->value.IsString() && idIt != input.MemberEnd() &&
                        idIt->value.IsUint())
                    {
                        // Check class
                        if (crc32(classIt->value.GetString(), classIt->value.GetStringLength()) ==
                            CRC32(DEVICE_ID_TYPE_NAME))
                        {
                            id = (uint8_t)idIt->value.GetUint();

                            return true;
                        }
                    }
                }
                break;
            }
            case ValueType::kServiceIDType:
            {
                identifier_t& id = *(identifier_t*)value;

                if (input.IsObject())
                {
                    rapidjson::Value::MemberIterator classIt = input.FindMember("_class");
                    rapidjson::Value::MemberIterator idIt = input.FindMember("id");
                    if (classIt != input.MemberEnd() && classIt->value.IsString() && idIt != input.MemberEnd() &&
                        idIt->value.IsUint())
                    {
                        // Check class
                        if (crc32(classIt->value.GetString(), classIt->value.GetStringLength()) ==
                            CRC32(SERVICE_ID_TYPE_NAME))
                        {
                            id = (uint8_t)idIt->value.GetUint();

                            return true;
                        }
                    }
                }
                break;
            }
            default:
                return false;
            }

            return true;
        }

        std::string Value::ToString()
        {
            switch (type)
            {
            case ValueType::kBooleanType:
                return std::to_string(GetBoolean());
            case ValueType::kNumberType:
                return std::to_string(GetNumber());
            case ValueType::kStringType:
                return GetString();
            case ValueType::kEndpointType:
            {
                std::stringstream ss;

                const Endpoint& endpoint = GetEndpoint();

                ss << "host: " << endpoint.host << std::endl;
                ss << "port: " << endpoint.port;

                return ss.str();
            }
            case ValueType::kColorType:
            {
                std::stringstream ss;

                const Color& color = GetColor();

                ss << "red  : " << (size_t)color.red << std::endl;
                ss << "green: " << (size_t)color.green << std::endl;
                ss << "blue : " << (size_t)color.blue;

                return ss.str();
            }
            case ValueType::kRoomIDType:
            {
                std::stringstream ss;

                ss << "room id: " << GetRoomID() << std::endl;

                return ss.str();
            }
            case ValueType::kDeviceIDType:
            {
                std::stringstream ss;

                ss << "device id: " << GetDeviceID() << std::endl;

                return ss.str();
            }
            case ValueType::kServiceIDType:
            {
                std::stringstream ss;

                ss << "service id: " << GetServiceID() << std::endl;

                return ss.str();
            }
            default:
                return "null";
            }
        }
    }
}