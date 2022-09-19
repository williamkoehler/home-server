#include "Property.hpp"

namespace server
{
    namespace scripting
    {
        std::string StringifyPropertyType(PropertyType type)
        {
            switch (type)
            {
            case PropertyType::kBooleanType:
                return "boolean";
            case PropertyType::kIntegerType:
                return "integer";
            case PropertyType::kNumberType:
                return "number";
            case PropertyType::kStringType:
                return "string";
            case PropertyType::kEndpointType:
                return "endpoint";
            case PropertyType::kColorType:
                return "color";
            default:
                return "unknown";
            }
        }
        PropertyType ParsePropertyType(const std::string& type)
        {
            switch (crc32(type.data(), type.size()))
            {
            case CRC32("boolean"):
                return PropertyType::kBooleanType;
            case CRC32("integer"):
                return PropertyType::kIntegerType;
            case CRC32("number"):
                return PropertyType::kNumberType;
            case CRC32("string"):
                return PropertyType::kStringType;
            case CRC32("endpoint"):
                return PropertyType::kEndpointType;
            case CRC32("color"):
                return PropertyType::kColorType;
            default:
                return PropertyType::kUnknownType;
            }
        }

        Ref<Property> Property::Create(PropertyType type)
        {
            switch (type)
            {
            case PropertyType::kBooleanType:
                return BooleanProperty::Create();
            case PropertyType::kIntegerType:
                return IntegerProperty::Create();
            case PropertyType::kNumberType:
                return NumberProperty::Create();
            case PropertyType::kStringType:
                return StringProperty::Create();
            case PropertyType::kEndpointType:
                return EndpointProperty::Create();
            case PropertyType::kColorType:
                return ColorProperty::Create();
            default:
                return nullptr;
            }
        }

        Ref<NullProperty> NullProperty::Create()
        {
            return boost::make_shared<NullProperty>();
        }
        PropertyType NullProperty::GetType() const
        {
            return PropertyType::kUnknownType;
        }

        //! Boolean
        Ref<BooleanProperty> BooleanProperty::Create()
        {
            return boost::make_shared<BooleanProperty>();
        }
        PropertyType BooleanProperty::GetType() const
        {
            return PropertyType::kBooleanType;
        }
        bool BooleanProperty::GetBoolean()
        {
            return value;
        }
        void BooleanProperty::SetBoolean(bool v)
        {
            value = v;
        }
        rapidjson::Value BooleanProperty::JsonGet(rapidjson::Document::AllocatorType& allocator)
        {
            return rapidjson::Value(value);
        }
        bool BooleanProperty::JsonSet(rapidjson::Value& input)
        {
            if (input.IsBool())
            {
                value = input.GetBool();

                return true;
            }

            return false;
        }

        //! Integer
        Ref<IntegerProperty> IntegerProperty::Create()
        {
            return boost::make_shared<IntegerProperty>();
        }
        PropertyType IntegerProperty::GetType() const
        {
            return PropertyType::kIntegerType;
        }
        int64_t IntegerProperty::GetInteger()
        {
            return value;
        }
        void IntegerProperty::SetInteger(int64_t v)
        {
            value = v;
        }
        double IntegerProperty::GetNumber()
        {
            return (double)value;
        }
        void IntegerProperty::SetNumber(double v)
        {
            value = (int64_t)v;
        }
        rapidjson::Value IntegerProperty::JsonGet(rapidjson::Document::AllocatorType& allocator)
        {
            return rapidjson::Value(value);
        }
        bool IntegerProperty::JsonSet(rapidjson::Value& input)
        {
            if (input.IsInt64())
            {
                value = input.GetInt64();

                return true;
            }

            return false;
        }

        //! Number
        Ref<NumberProperty> NumberProperty::Create()
        {
            return boost::make_shared<NumberProperty>();
        }
        PropertyType NumberProperty::GetType() const
        {
            return PropertyType::kNumberType;
        }
        int64_t NumberProperty::GetInteger()
        {
            return (int64_t)value;
        }
        void NumberProperty::SetInteger(int64_t v)
        {
            value = (double)v;
        }
        double NumberProperty::GetNumber()
        {
            return value;
        }
        void NumberProperty::SetNumber(double v)
        {
            value = v;
        }
        rapidjson::Value NumberProperty::JsonGet(rapidjson::Document::AllocatorType& allocator)
        {
            return rapidjson::Value(value);
        }
        bool NumberProperty::JsonSet(rapidjson::Value& input)
        {
            if (input.IsNumber())
            {
                value = input.GetDouble();

                return true;
            }

            return false;
        }

        //! String
        Ref<StringProperty> StringProperty::Create()
        {
            return boost::make_shared<StringProperty>();
        }
        PropertyType StringProperty::GetType() const
        {
            return PropertyType::kStringType;
        }
        std::string StringProperty::GetString()
        {
            return value;
        }
        void StringProperty::SetString(const std::string& v)
        {
            value = v;
        }
        rapidjson::Value StringProperty::JsonGet(rapidjson::Document::AllocatorType& allocator)
        {
            return rapidjson::Value(value.data(), value.size());
        }
        bool StringProperty::JsonSet(rapidjson::Value& input)
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
        Ref<EndpointProperty> EndpointProperty::Create()
        {
            return boost::make_shared<EndpointProperty>();
        }
        PropertyType EndpointProperty::GetType() const
        {
            return PropertyType::kEndpointType;
        }
        Endpoint EndpointProperty::GetEndpoint()
        {
            return value;
        }
        void EndpointProperty::SetEndpoint(const Endpoint& v)
        {
            value = v;
        }
        rapidjson::Value EndpointProperty::JsonGet(rapidjson::Document::AllocatorType& allocator)
        {
            rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);

            json.AddMember("_class", rapidjson::Value("endpoint"), allocator);
            json.AddMember("host", rapidjson::Value(value.host.data(), value.host.size(), allocator), allocator);
            json.AddMember("port", rapidjson::Value(value.port), allocator);

            return json;
        }
        bool EndpointProperty::JsonSet(rapidjson::Value& input)
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
        Ref<ColorProperty> ColorProperty::Create()
        {
            return boost::make_shared<ColorProperty>();
        }
        PropertyType ColorProperty::GetType() const
        {
            return PropertyType::kColorType;
        }
        Color ColorProperty::GetColor()
        {
            return value;
        }
        void ColorProperty::SetColor(const Color& v)
        {
            value = v;
        }
        rapidjson::Value ColorProperty::JsonGet(rapidjson::Document::AllocatorType& allocator)
        {
            rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);

            json.AddMember("_class", rapidjson::Value("color"), allocator);
            json.AddMember("r", rapidjson::Value(value.red), allocator);
            json.AddMember("g", rapidjson::Value(value.green), allocator);
            json.AddMember("b", rapidjson::Value(value.blue), allocator);

            return json;
        }
        bool ColorProperty::JsonSet(rapidjson::Value& input)
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