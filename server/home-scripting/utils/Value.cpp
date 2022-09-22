#include "Value.hpp"

namespace server
{
    namespace scripting
    {
        std::string StringifyValueType(ValueType type)
        {
            switch (type)
            {
            case ValueType::kBooleanType:
                return "boolean";
            case ValueType::kNumberType:
                return "number";
            case ValueType::kStringType:
                return "string";
            case ValueType::kEndpointType:
                return "endpoint";
            case ValueType::kColorType:
                return "color";
            default:
                return "unknown";
            }
        }
        ValueType ParseValueType(const std::string& type)
        {
            switch (crc32(type.data(), type.size()))
            {
            case CRC32("boolean"):
                return ValueType::kBooleanType;
            case CRC32("number"):
                return ValueType::kNumberType;
            case CRC32("string"):
                return ValueType::kStringType;
            case CRC32("endpoint"):
                return ValueType::kEndpointType;
            case CRC32("color"):
                return ValueType::kColorType;
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
            switch (type)
            {
            case ValueType::kBooleanType:
                return boost::make_shared<Value>(false);
            case ValueType::kNumberType:
                return boost::make_shared<Value>(0.0);
            case ValueType::kStringType:
                return boost::make_shared<Value>(std::string());
            case ValueType::kEndpointType:
                return boost::make_shared<Value>(Endpoint{});
            case ValueType::kColorType:
                return boost::make_shared<Value>(Color{});
            default:
                return boost::make_shared<Value>();
            }
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
                    case CRC32("endpoint"):
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
                    case CRC32("color"):
                    {
                        rapidjson::Value::MemberIterator rIt = input.FindMember("r");
                        rapidjson::Value::MemberIterator gIt = input.FindMember("g");
                        rapidjson::Value::MemberIterator bIt = input.FindMember("b");
                        if (classIt != input.MemberEnd() && classIt->value.IsString() && rIt != input.MemberEnd() &&
                            rIt->value.IsUint() && gIt != input.MemberEnd() && gIt->value.IsUint() &&
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
                        if (crc32(classIt->value.GetString(), classIt->value.GetStringLength()) == CRC32("endpoint"))
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
                        if (crc32(classIt->value.GetString(), classIt->value.GetStringLength()) == CRC32("color"))
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
            default:
                return false;
            }

            return true;
        }
    }
}