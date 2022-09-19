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
            case ValueType::kIntegerType:
                return "integer";
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
            case CRC32("integer"):
                return ValueType::kIntegerType;
            case CRC32("number"):
                return ValueType::kNumberType;
            case CRC32("string"):
                return ValueType::kStringType;
            case CRC32("endpoint"):
                return ValueType::kEndpointType;
            case CRC32("color"):
                return ValueType::kColorType;
            default:
                return ValueType::kUnknownType;
            }
        }

        Ref<Value> Value::Create(ValueType type)
        {
            switch (type)
            {
            case ValueType::kBooleanType:
                return BooleanValue::Create();
            case ValueType::kIntegerType:
                return IntegerValue::Create();
            case ValueType::kNumberType:
                return NumberValue::Create();
            case ValueType::kStringType:
                return StringValue::Create();
            case ValueType::kEndpointType:
                return EndpointValue::Create();
            case ValueType::kColorType:
                return ColorValue::Create();
            default:
                return nullptr;
            }
        }

        Ref<NullValue> NullValue::Create()
        {
            return boost::make_shared<NullValue>();
        }
        ValueType NullValue::GetType() const
        {
            return ValueType::kUnknownType;
        }

        //! Boolean
        Ref<BooleanValue> BooleanValue::Create()
        {
            return boost::make_shared<BooleanValue>();
        }
        ValueType BooleanValue::GetType() const
        {
            return ValueType::kBooleanType;
        }
        bool BooleanValue::GetBoolean()
        {
            return value;
        }
        void BooleanValue::SetBoolean(bool v)
        {
            value = v;
        }
        rapidjson::Value BooleanValue::JsonGet(rapidjson::Document::AllocatorType& allocator)
        {
            return rapidjson::Value(value);
        }
        bool BooleanValue::JsonSet(rapidjson::Value& input)
        {
            if (input.IsBool())
            {
                value = input.GetBool();

                return true;
            }

            return false;
        }

        //! Integer
        Ref<IntegerValue> IntegerValue::Create()
        {
            return boost::make_shared<IntegerValue>();
        }
        ValueType IntegerValue::GetType() const
        {
            return ValueType::kIntegerType;
        }
        int64_t IntegerValue::GetInteger()
        {
            return value;
        }
        void IntegerValue::SetInteger(int64_t v)
        {
            value = v;
        }
        double IntegerValue::GetNumber()
        {
            return (double)value;
        }
        void IntegerValue::SetNumber(double v)
        {
            value = (int64_t)v;
        }
        rapidjson::Value IntegerValue::JsonGet(rapidjson::Document::AllocatorType& allocator)
        {
            return rapidjson::Value(value);
        }
        bool IntegerValue::JsonSet(rapidjson::Value& input)
        {
            if (input.IsInt64())
            {
                value = input.GetInt64();

                return true;
            }

            return false;
        }

        //! Number
        Ref<NumberValue> NumberValue::Create()
        {
            return boost::make_shared<NumberValue>();
        }
        ValueType NumberValue::GetType() const
        {
            return ValueType::kNumberType;
        }
        int64_t NumberValue::GetInteger()
        {
            return (int64_t)value;
        }
        void NumberValue::SetInteger(int64_t v)
        {
            value = (double)v;
        }
        double NumberValue::GetNumber()
        {
            return value;
        }
        void NumberValue::SetNumber(double v)
        {
            value = v;
        }
        rapidjson::Value NumberValue::JsonGet(rapidjson::Document::AllocatorType& allocator)
        {
            return rapidjson::Value(value);
        }
        bool NumberValue::JsonSet(rapidjson::Value& input)
        {
            if (input.IsNumber())
            {
                value = input.GetDouble();

                return true;
            }

            return false;
        }

        //! String
        Ref<StringValue> StringValue::Create()
        {
            return boost::make_shared<StringValue>();
        }
        ValueType StringValue::GetType() const
        {
            return ValueType::kStringType;
        }
        std::string StringValue::GetString()
        {
            return value;
        }
        void StringValue::SetString(const std::string& v)
        {
            value = v;
        }
        rapidjson::Value StringValue::JsonGet(rapidjson::Document::AllocatorType& allocator)
        {
            return rapidjson::Value(value.data(), value.size());
        }
        bool StringValue::JsonSet(rapidjson::Value& input)
        {
            if (input.IsString())
            {
                size_t length = input.GetStringLength();
                value = std::string(input.GetString(), length);

                return true;
            }

            return false;
        }

        //! Endpoint
        Ref<EndpointValue> EndpointValue::Create()
        {
            return boost::make_shared<EndpointValue>();
        }
        ValueType EndpointValue::GetType() const
        {
            return ValueType::kEndpointType;
        }
        Endpoint EndpointValue::GetEndpoint()
        {
            return value;
        }
        void EndpointValue::SetEndpoint(const Endpoint& v)
        {
            value = v;
        }
        rapidjson::Value EndpointValue::JsonGet(rapidjson::Document::AllocatorType& allocator)
        {
            rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);

            json.AddMember("_class", rapidjson::Value("endpoint"), allocator);
            json.AddMember("host", rapidjson::Value(value.host.data(), value.host.size(), allocator), allocator);
            json.AddMember("port", rapidjson::Value(value.port), allocator);

            return json;
        }
        bool EndpointValue::JsonSet(rapidjson::Value& input)
        {
            if (input.IsObject())
            {
                rapidjson::Value::MemberIterator classIt = input.FindMember("_class");
                rapidjson::Value::MemberIterator hostIt = input.FindMember("host");
                rapidjson::Value::MemberIterator portIt = input.FindMember("port");
                if (classIt != input.MemberEnd() && classIt->value.IsString() && hostIt != input.MemberEnd() &&
                    hostIt->value.IsString() && portIt != input.MemberEnd() && portIt->value.IsUint())
                {
                    // Check class
                    if (crc32(input.GetString(), input.GetStringLength()) == CRC32("endpoint"))
                    {
                        size_t length = hostIt->value.GetStringLength();
                        value.host = std::string(hostIt->value.GetString(), length);

                        value.port = (uint16_t)portIt->value.GetUint();

                        return true;
                    }
                }
            }

            return false;
        }

        //! Color
        Ref<ColorValue> ColorValue::Create()
        {
            return boost::make_shared<ColorValue>();
        }
        ValueType ColorValue::GetType() const
        {
            return ValueType::kColorType;
        }
        Color ColorValue::GetColor()
        {
            return value;
        }
        void ColorValue::SetColor(const Color& v)
        {
            value = v;
        }
        rapidjson::Value ColorValue::JsonGet(rapidjson::Document::AllocatorType& allocator)
        {
            rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);

            json.AddMember("_class", rapidjson::Value("color"), allocator);
            json.AddMember("r", rapidjson::Value(value.red), allocator);
            json.AddMember("g", rapidjson::Value(value.green), allocator);
            json.AddMember("b", rapidjson::Value(value.blue), allocator);

            return json;
        }
        bool ColorValue::JsonSet(rapidjson::Value& input)
        {
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
                    if (crc32(input.GetString(), input.GetStringLength()) == CRC32("color"))
                    {
                        value.red = (uint8_t)rIt->value.GetUint();
                        value.green = (uint8_t)gIt->value.GetUint();
                        value.blue = (uint8_t)bIt->value.GetUint();

                        return true;
                    }
                }
            }

            return false;
        }
    }
}