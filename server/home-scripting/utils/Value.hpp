#pragma once
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        /// @brief Endpoint object (host:port)
        ///
        struct Endpoint
        {
            std::string host;
            uint16_t port;
        };

        /// @brief Color object (red;green;blue)
        ///
        struct Color
        {
            uint8_t red, green, blue;
        };

        enum ValueType : uint8_t
        {
            kUnknownType,
            kNullType,
            kBooleanType,
            kNumberType,
            kStringType,
            kEndpointType,
            kColorType,
        };

        std::string_view StringifyValueTypeConst(ValueType type);
        inline std::string StringifyValueType(ValueType type)
        {
            return std::string(StringifyValueTypeConst(type));
        }
        ValueType ParseValueType(const std::string_view& type);
        inline ValueType ParseValueType(const std::string& type)
        {
            return ParseValueType(std::string_view(type));
        }

        class Value
        {
          private:
            const ValueType type;

            /// @brief Calculate value array size
            ///
            /// @tparam args Types
            template <typename... args>
            struct static_max;

            template <typename arg>
            struct static_max<arg>
            {
                static const size_t value = sizeof(arg);
            };

            template <typename arg1, typename arg2, typename... args>
            struct static_max<arg1, arg2, args...>
            {
                static const size_t value =
                    sizeof(arg1) >= sizeof(arg2) ? static_max<arg1, args...>::value : static_max<arg2, args...>::value;
            };

            uint8_t value[static_max<bool, double_t, std::string, Endpoint, Color>::value];

          public:
            explicit Value();
            explicit Value(bool boolean);
            explicit Value(double_t number);
            explicit Value(const std::string& string);
            explicit Value(const std::string_view& string);
            explicit Value(const Endpoint& endpoint);
            explicit Value(const Color& color);
            ~Value();
            static Ref<Value> Create(ValueType type);
            inline static Ref<Value> CreateNull()
            {
                return boost::make_shared<Value>();
            }
            inline static Ref<Value> CreateBoolean(bool boolean = false)
            {
                return boost::make_shared<Value>(boolean);
            }
            inline static Ref<Value> CreateInteger(int64_t integer = 0l)
            {
                return boost::make_shared<Value>((double_t)integer);
            }
            inline static Ref<Value> CreateNumber(double_t number = 0.0)
            {
                return boost::make_shared<Value>(number);
            }
            inline static Ref<Value> CreateString(const std::string& string = std::string())
            {
                return boost::make_shared<Value>(string);
            }
            inline static Ref<Value> CreateStringView(const std::string_view& string = std::string_view())
            {
                return boost::make_shared<Value>(string);
            }
            inline static Ref<Value> CreateEndpoint(const Endpoint& endpoint = Endpoint{})
            {
                return boost::make_shared<Value>(endpoint);
            }
            inline static Ref<Value> CreateColor(const Color& color = Color{})
            {
                return boost::make_shared<Value>(color);
            }
            static Ref<Value> Create(rapidjson::Value& json);

            inline ValueType GetType() const
            {
                return type;
            };

            inline bool IsNull() const
            {
                return type == ValueType::kNullType;
            }
            inline bool IsBoolean() const
            {
                return type == ValueType::kBooleanType;
            }
            inline bool IsInteger() const
            {
                return type == ValueType::kNumberType;
            }
            inline bool IsNumber() const
            {
                return type == ValueType::kNumberType;
            }
            inline bool IsString() const
            {
                return type == ValueType::kStringType;
            }
            inline bool IsEndpoint() const
            {
                return type == ValueType::kEndpointType;
            }
            inline bool IsColor() const
            {
                return type == ValueType::kColorType;
            }

            inline bool GetBoolean()
            {
                assert(type == ValueType::kBooleanType);
                return *(bool*)value;
            }
            inline int64_t GetInteger()
            {
                assert(type == ValueType::kNumberType);
                return (int64_t) * (double_t*)value;
            }
            inline double_t GetNumber()
            {
                assert(type == ValueType::kNumberType);
                return *(double_t*)value;
            }
            inline const std::string& GetString()
            {
                assert(type == ValueType::kStringType);
                return *(std::string*)value;
            }
            inline const Endpoint& GetEndpoint()
            {
                assert(type == ValueType::kEndpointType);
                return *(Endpoint*)value;
            }
            inline const Color& GetColor()
            {
                assert(type == ValueType::kColorType);
                return *(Color*)value;
            }

            inline void SetBoolean(bool v)
            {
                assert(type == ValueType::kBooleanType);
                *(bool*)value = v;
            }
            inline void SetInteger(int64_t v)
            {
                assert(type == ValueType::kNumberType);
                *(int64_t*)value = v;
            }
            inline void SetNumber(double_t v)
            {
                assert(type == ValueType::kNumberType);
                *(double_t*)value = v;
            }
            inline void SetString(const std::string& v)
            {
                assert(type == ValueType::kStringType);
                *(std::string*)value = v;
            }
            inline void SetStringView(const std::string_view& v)
            {
                assert(type == ValueType::kStringType);
                *(std::string*)value = v;
            }
            inline void SetEndpoint(const Endpoint& v)
            {
                assert(type == ValueType::kEndpointType);

                Endpoint& endpoint = *(Endpoint*)value;
                endpoint.host = v.host;
                endpoint.port = v.port;
            }
            inline void SetColor(const Color& v)
            {
                assert(type == ValueType::kColorType);

                Color& color = *(Color*)value;
                color.red = v.red;
                color.green = v.green;
                color.blue = v.blue;
            }

            std::string ToString();

            rapidjson::Value JsonGet(rapidjson::Document::AllocatorType& allocator);
            bool JsonSet(rapidjson::Value& input);
        };
    }
}