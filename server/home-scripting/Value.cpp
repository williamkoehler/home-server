#include "Value.hpp"

#define BOOLEAN_TYPE_NAME "boolean"
#define INTEGER_TYPE_NAME "integer"
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
            case ValueType::kIntegerType:
                return INTEGER_TYPE_NAME;
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
            case CRC32(INTEGER_TYPE_NAME):
                return ValueType::kIntegerType;
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
            new ((void*)value.data()) bool(boolean);
        }
        Value::Value(ssize_t integer) : type(ValueType::kIntegerType)
        {
            new ((void*)value.data()) ssize_t(integer);
        }
        Value::Value(double_t number) : type(ValueType::kNumberType)
        {
            new ((void*)value.data()) double_t(number);
        }
        Value::Value(const std::string& string) : type(ValueType::kStringType)
        {
            new ((void*)value.data()) std::string(string);
        }
        Value::Value(const std::string_view& string) : type(ValueType::kStringType)
        {
            new ((void*)value.data()) std::string(string);
        }
        Value::Value(const Endpoint& endpoint) : type(ValueType::kEndpointType)
        {
            new ((void*)value.data()) Endpoint(endpoint);
        }
        Value::Value(const Color& color) : type(ValueType::kColorType)
        {
            new ((void*)value.data()) Color(color);
        }
        Value::Value(ValueType type) : type(type)
        {
            switch (type)
            {
            case ValueType::kBooleanType:
                new ((void*)value.data()) bool();
                break;
            case ValueType::kIntegerType:
                new ((void*)value.data()) ssize_t();
                break;
            case ValueType::kNumberType:
                new ((void*)value.data()) double_t();
                break;
            case ValueType::kStringType:
                new ((void*)value.data()) std::string();
                break;
            case ValueType::kEndpointType:
                new ((void*)value.data()) Endpoint();
                break;
            case ValueType::kColorType:
                new ((void*)value.data()) Color();
                break;
            case ValueType::kRoomIDType:
            case ValueType::kDeviceIDType:
            case ValueType::kServiceIDType:
                new ((void*)value.data()) identifier_t();
                break;
            default:
                break;
            }
        }
        Value::Value(const Value& other) : type(other.type)
        {
            switch (type)
            {
            case ValueType::kBooleanType:
                new ((void*)value.data()) bool(*(bool*)other.value.data());
                break;
            case ValueType::kIntegerType:
                new ((void*)value.data()) ssize_t(*(ssize_t*)other.value.data());
                break;
            case ValueType::kNumberType:
                new ((void*)value.data()) double_t(*(double_t*)other.value.data());
                break;
            case ValueType::kStringType:
                new ((void*)value.data()) std::string(*(std::string*)other.value.data());
                break;
            case ValueType::kEndpointType:
                new ((void*)value.data()) Endpoint(*(Endpoint*)other.value.data());
                break;
            case ValueType::kColorType:
                new ((void*)value.data()) Color(*(Color*)other.value.data());
                break;
            case ValueType::kRoomIDType:
            case ValueType::kDeviceIDType:
            case ValueType::kServiceIDType:
                new ((void*)value.data()) identifier_t(*(identifier_t*)other.value.data());
                break;
            default:
                break;
            }
        }
        Value::Value(Value&& other) noexcept
            : type(std::exchange(other.type, ValueType::kUnknownType)), value(other.value)
        {
        }
        Value::~Value()
        {
            switch (type)
            {
            case ValueType::kStringType:
                ((std::string*)value.data())->~basic_string();
                break;
            case ValueType::kEndpointType:
                ((Endpoint*)value.data())->~Endpoint();
                break;
            case ValueType::kColorType:
                ((Color*)value.data())->~Color();
                break;
            default: // Do nothing for basic types such as bool, int, etc...
                break;
            }
        }

        Value Value::Create(const rapidjson::Value& input)
        {
            switch (input.GetType())
            {
            case rapidjson::kTrueType:
                return Value(true);
            case rapidjson::kFalseType:
                return Value(false);
            case rapidjson::kNumberType:
                if (input.IsInt64())
                    return Value(input.GetInt64());
                else
                    return Value(input.GetDouble());
            case rapidjson::kStringType:
                return Value(std::string_view(input.GetString(), input.GetStringLength()));
            case rapidjson::kObjectType:
            {
                rapidjson::Value::ConstMemberIterator classIt = input.FindMember("_class");
                if (classIt != input.MemberEnd() && classIt->value.IsString())
                {
                    switch (crc32(classIt->value.GetString(), classIt->value.GetStringLength()))
                    {
                    case CRC32(ENDPOINT_TYPE_NAME):
                    {
                        rapidjson::Value::ConstMemberIterator hostIt = input.FindMember("host");
                        rapidjson::Value::ConstMemberIterator portIt = input.FindMember("port");
                        if (hostIt != input.MemberEnd() && hostIt->value.IsString() && portIt != input.MemberEnd() &&
                            portIt->value.IsUint())
                        {
                            Endpoint endpoint = Endpoint{
                                .host = std::string(hostIt->value.GetString(), hostIt->value.GetStringLength()),
                                .port = (uint16_t)portIt->value.GetUint(),
                            };

                            return Value(endpoint);
                        }
                    }
                    case CRC32(COLOR_TYPE_NAME):
                    {
                        rapidjson::Value::ConstMemberIterator rIt = input.FindMember("r");
                        rapidjson::Value::ConstMemberIterator gIt = input.FindMember("g");
                        rapidjson::Value::ConstMemberIterator bIt = input.FindMember("b");
                        if (rIt->value.IsUint() && gIt != input.MemberEnd() && gIt->value.IsUint() &&
                            bIt != input.MemberEnd() && bIt->value.IsUint())
                        {
                            Color color = Color{
                                .red = (uint8_t)rIt->value.GetUint(),
                                .green = (uint8_t)gIt->value.GetUint(),
                                .blue = (uint8_t)bIt->value.GetUint(),
                            };

                            return Value(color);
                        }
                    }
                    case CRC32(ROOM_ID_TYPE_NAME):
                    {
                        rapidjson::Value::ConstMemberIterator idIt = input.FindMember("id");
                        if (idIt != input.MemberEnd() && idIt->value.IsUint())
                            return Value::Create<ValueType::kRoomIDType>(idIt->value.GetUint());
                    }
                    case CRC32(DEVICE_ID_TYPE_NAME):
                    {
                        rapidjson::Value::ConstMemberIterator idIt = input.FindMember("id");
                        if (idIt != input.MemberEnd() && idIt->value.IsUint())
                            return Value::Create<ValueType::kDeviceIDType>(idIt->value.GetUint());
                    }
                    case CRC32(SERVICE_ID_TYPE_NAME):
                    {
                        rapidjson::Value::ConstMemberIterator idIt = input.FindMember("id");
                        if (idIt != input.MemberEnd() && idIt->value.IsUint())
                            return Value::Create<ValueType::kServiceIDType>(idIt->value.GetUint());
                    }
                    }
                }

                break;
            }
            default:
                break;
            }

            return Value();
        }

        void Value::operator=(const Value& other) noexcept
        {
            // Call deconstructor on old value
            this->~Value();

            type = other.type;
            switch (type)
            {
            case ValueType::kBooleanType:
                new ((void*)value.data()) bool(*(bool*)other.value.data());
                break;
            case ValueType::kIntegerType:
                new ((void*)value.data()) ssize_t(*(ssize_t*)other.value.data());
                break;
            case ValueType::kNumberType:
                new ((void*)value.data()) double_t(*(double_t*)other.value.data());
                break;
            case ValueType::kStringType:
                new ((void*)value.data()) std::string(*(std::string*)other.value.data());
                break;
            case ValueType::kEndpointType:
                new ((void*)value.data()) Endpoint(*(Endpoint*)other.value.data());
                break;
            case ValueType::kColorType:
                new ((void*)value.data()) Color(*(Color*)other.value.data());
                break;
            case ValueType::kRoomIDType:
            case ValueType::kDeviceIDType:
            case ValueType::kServiceIDType:
                new ((void*)value.data()) identifier_t(*(identifier_t*)other.value.data());
                break;
            default:
                break;
            }
        }

        void Value::operator=(Value&& other) noexcept
        {
            // Call deconstructor on old value
            this->~Value();

            type = std::exchange(other.type, ValueType::kUnknownType);
            value = other.value;
        }

        rapidjson::Value Value::JsonGet(rapidjson::Document::AllocatorType& allocator) const
        {
            switch (type)
            {
            case ValueType::kBooleanType:
                return rapidjson::Value(*(bool*)value.data());
            case ValueType::kIntegerType:
                return rapidjson::Value(*(ssize_t*)value.data());
            case ValueType::kNumberType:
                return rapidjson::Value(*(double_t*)value.data());
            case ValueType::kStringType:
            {
                const std::string& string = *(std::string*)value.data();
                return rapidjson::Value(string.data(), string.size(), allocator);
            }
            case ValueType::kEndpointType:
            {
                const Endpoint& endpoint = *(Endpoint*)value.data();

                rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);

                json.AddMember("_class", rapidjson::Value("endpoint"), allocator);
                json.AddMember("host", rapidjson::Value(endpoint.host.data(), endpoint.host.size(), allocator),
                               allocator);
                json.AddMember("port", rapidjson::Value(endpoint.port), allocator);

                return json;
            }
            case ValueType::kColorType:
            {
                const Color& color = *(Color*)value.data();

                rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);

                json.AddMember("_class", rapidjson::Value("color"), allocator);
                json.AddMember("r", rapidjson::Value(color.red), allocator);
                json.AddMember("g", rapidjson::Value(color.green), allocator);
                json.AddMember("b", rapidjson::Value(color.blue), allocator);

                return json;
            }
            case ValueType::kRoomIDType:
            {
                const identifier_t& id = *(identifier_t*)value.data();

                rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);

                json.AddMember("_class", rapidjson::Value("room"), allocator);
                json.AddMember("id", rapidjson::Value(id), allocator);

                return json;
            }
            case ValueType::kDeviceIDType:
            {
                const identifier_t& id = *(identifier_t*)value.data();

                rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);

                json.AddMember("_class", rapidjson::Value("device"), allocator);
                json.AddMember("id", rapidjson::Value(id), allocator);

                return json;
            }
            case ValueType::kServiceIDType:
            {
                const identifier_t& id = *(identifier_t*)value.data();

                rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);

                json.AddMember("_class", rapidjson::Value("service"), allocator);
                json.AddMember("id", rapidjson::Value(id), allocator);

                return json;
            }
            default:
                return rapidjson::Value(rapidjson::kNullType);
            }
        }

        bool Value::JsonSet(const rapidjson::Value& input)
        {
            switch (type)
            {
            case ValueType::kBooleanType:
                if (input.IsBool())
                    *((bool*)value.data()) = input.GetBool();
                break;
            case ValueType::kIntegerType:
                if (input.IsInt64())
                    *((ssize_t*)value.data()) = input.GetInt64();
                break;
            case ValueType::kNumberType:
                if (input.IsNumber())
                    *((bool*)value.data()) = input.GetDouble();
                break;
            case ValueType::kStringType:
            {
                std::string& string = *(std::string*)value.data();

                if (input.IsString())
                    string.assign(input.GetString(), input.GetStringLength());
                break;
            }
            case ValueType::kEndpointType:
            {
                Endpoint& endpoint = *(Endpoint*)value.data();

                if (input.IsObject())
                {
                    rapidjson::Value::ConstMemberIterator classIt = input.FindMember("_class");
                    rapidjson::Value::ConstMemberIterator hostIt = input.FindMember("host");
                    rapidjson::Value::ConstMemberIterator portIt = input.FindMember("port");
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
                Color& color = *(Color*)value.data();

                if (input.IsObject())
                {
                    rapidjson::Value::ConstMemberIterator classIt = input.FindMember("_class");
                    rapidjson::Value::ConstMemberIterator rIt = input.FindMember("r");
                    rapidjson::Value::ConstMemberIterator gIt = input.FindMember("g");
                    rapidjson::Value::ConstMemberIterator bIt = input.FindMember("b");
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
                identifier_t& id = *(identifier_t*)value.data();

                if (input.IsObject())
                {
                    rapidjson::Value::ConstMemberIterator classIt = input.FindMember("_class");
                    rapidjson::Value::ConstMemberIterator idIt = input.FindMember("id");
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
                identifier_t& id = *(identifier_t*)value.data();

                if (input.IsObject())
                {
                    rapidjson::Value::ConstMemberIterator classIt = input.FindMember("_class");
                    rapidjson::Value::ConstMemberIterator idIt = input.FindMember("id");
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
                identifier_t& id = *(identifier_t*)value.data();

                if (input.IsObject())
                {
                    rapidjson::Value::ConstMemberIterator classIt = input.FindMember("_class");
                    rapidjson::Value::ConstMemberIterator idIt = input.FindMember("id");
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

        void Value::Assign(const Value& other)
        {
            {
                switch (type)
                {
                case ValueType::kBooleanType:
                    if (other.type == ValueType::kBooleanType)
                        *((bool*)value.data()) = *(bool*)other.value.data();
                    else
                        *((bool*)value.data()) = false;
                    break;
                case ValueType::kIntegerType:
                    if (other.type == ValueType::kIntegerType)
                        *((ssize_t*)value.data()) = *(ssize_t*)other.value.data();
                    if (other.type == ValueType::kNumberType)
                        *((ssize_t*)value.data()) = (ssize_t)(*(double_t*)other.value.data());
                    else
                        *((ssize_t*)value.data()) = 0.0;
                    break;
                case ValueType::kNumberType:
                    if (other.type == ValueType::kNumberType)
                        *((double_t*)value.data()) = *(double_t*)other.value.data();
                    if (other.type == ValueType::kIntegerType)
                        *((double_t*)value.data()) = (double_t)(*(ssize_t*)other.value.data());
                    else
                        *((double_t*)value.data()) = 0.0;
                    break;
                case ValueType::kStringType:
                    if (other.type == ValueType::kStringType)
                        *((std::string*)value.data()) = *(std::string*)other.value.data();
                    else
                        ((std::string*)value.data())->clear();
                    break;
                case ValueType::kEndpointType:
                    if (other.type == ValueType::kEndpointType)
                        *((Endpoint*)value.data()) = *(Endpoint*)other.value.data();
                    else
                        *((Endpoint*)value.data()) = Endpoint{.host = "", .port = 0};
                    break;
                case ValueType::kColorType:
                    if (other.type == ValueType::kBooleanType)
                        *((Color*)value.data()) = *(Color*)other.value.data();
                    else
                        *((Color*)value.data()) = Color{.red = 0, .green = 0, .blue = 0};
                    break;
                case ValueType::kRoomIDType:
                case ValueType::kDeviceIDType:
                case ValueType::kServiceIDType:
                    if (other.type == ValueType::kBooleanType)
                        *((identifier_t*)value.data()) = *(identifier_t*)other.value.data();
                    else
                        *((identifier_t*)value.data()) = 0;
                    break;
                default:
                    break;
                }
            }
        }

        std::string Value::ToString()
        {
            switch (type)
            {
            case ValueType::kBooleanType:
                return std::to_string(GetBoolean());
            case ValueType::kIntegerType:
                return std::to_string(GetInteger());
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