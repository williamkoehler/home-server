#pragma once
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        /// @brief Void object
        ///
        struct Void
        {
        };

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

            kRoomIDType,
            kDeviceIDType,
            kServiceIDType,
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
            ValueType type;

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

            uint8_t value[static_max<bool, double_t, std::string, Endpoint, Color, identifier_t>::value];

          public:
            explicit Value();
            explicit Value(bool boolean);
            explicit Value(double_t number);
            explicit Value(const std::string& string);
            explicit Value(const std::string_view& string);
            explicit Value(const Endpoint& endpoint);
            explicit Value(const Color& color);
            explicit Value(ValueType type);
            Value(const Value& other);
            Value(Value&& other);
            ~Value();

            /// @brief Create value
            ///
            /// @tparam type Value type
            /// @tparam T Value
            /// @return Value
            template <ValueType type, typename T>
            static inline Value Create(const T&);

            /// @brief Create value from json
            ///
            /// @param json Json
            /// @return Value
            static Value Create(rapidjson::Value& json);

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
            inline bool IsRoomID() const
            {
                return type == ValueType::kRoomIDType;
            }
            inline bool IsDeviceID() const
            {
                return type == ValueType::kDeviceIDType;
            }
            inline bool IsServiceID() const
            {
                return type == ValueType::kServiceIDType;
            }

            inline bool& GetBoolean()
            {
                assert(type == ValueType::kBooleanType);
                return *(bool*)value;
            }
            inline const bool& GetBoolean() const
            {
                assert(type == ValueType::kBooleanType);
                return *(bool*)value;
            }

            inline int64_t GetInteger() const
            {
                assert(type == ValueType::kNumberType);
                return (int64_t) * (double_t*)value;
            }

            inline double_t& GetNumber()
            {
                assert(type == ValueType::kNumberType);
                return *(double_t*)value;
            }
            inline const double_t& GetNumber() const
            {
                assert(type == ValueType::kNumberType);
                return *(double_t*)value;
            }

            inline std::string& GetString()
            {
                assert(type == ValueType::kStringType);
                return *(std::string*)value;
            }
            inline const std::string& GetString() const
            {
                assert(type == ValueType::kStringType);
                return *(std::string*)value;
            }

            inline Endpoint& GetEndpoint()
            {
                assert(type == ValueType::kEndpointType);
                return *(Endpoint*)value;
            }
            inline const Endpoint& GetEndpoint() const
            {
                assert(type == ValueType::kEndpointType);
                return *(Endpoint*)value;
            }

            inline Color& GetColor()
            {
                assert(type == ValueType::kColorType);
                return *(Color*)value;
            }
            inline const Color& GetColor() const
            {
                assert(type == ValueType::kColorType);
                return *(Color*)value;
            }

            inline identifier_t& GetRoomID()
            {
                assert(type == ValueType::kRoomIDType);
                return *(identifier_t*)value;
            }
            inline const identifier_t& GetRoomID() const
            {
                assert(type == ValueType::kRoomIDType);
                return *(identifier_t*)value;
            }

            inline identifier_t& GetDeviceID()
            {
                assert(type == ValueType::kDeviceIDType);
                return *(identifier_t*)value;
            }
            inline const identifier_t& GetDeviceID() const
            {
                assert(type == ValueType::kDeviceIDType);
                return *(identifier_t*)value;
            }

            inline identifier_t& GetServiceID()
            {
                assert(type == ValueType::kServiceIDType);
                return *(identifier_t*)value;
            }
            inline const identifier_t& GetServiceID() const
            {
                assert(type == ValueType::kServiceIDType);
                return *(identifier_t*)value;
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
            inline void SetRoomID(const identifier_t& v)
            {
                assert(type == ValueType::kRoomIDType);
                *(identifier_t*)value = v;
            }
            inline void SetDeviceID(const identifier_t& v)
            {
                assert(type == ValueType::kDeviceIDType);
                *(identifier_t*)value = v;
            }
            inline void SetServiceID(const identifier_t& v)
            {
                assert(type == ValueType::kServiceIDType);
                *(identifier_t*)value = v;
            }

            std::string ToString();

            rapidjson::Value JsonGet(rapidjson::Document::AllocatorType& allocator);
            bool JsonSet(rapidjson::Value& input);
        };

        template <>
        inline Value Value::Create<ValueType::kBooleanType, bool>(const bool& boolean)
        {
            return Value(boolean);
        }

        template <>
        inline Value Value::Create<ValueType::kNumberType, double_t>(const double_t& number)
        {
            return Value(number);
        }

        template <>
        inline Value Value::Create<ValueType::kStringType, std::string>(const std::string& string)
        {
            return Value(string);
        }

        template <>
        inline Value Value::Create<ValueType::kEndpointType, Endpoint>(const Endpoint& endpoint)
        {
            return Value(endpoint);
        }

        template <>
        inline Value Value::Create<ValueType::kColorType, Color>(const Color& color)
        {
            return Value(color);
        }

        template <>
        inline Value Value::Create<ValueType::kRoomIDType, identifier_t>(const identifier_t& roomID)
        {
            Value value = Value(ValueType::kRoomIDType);
            value.SetRoomID(roomID);
            return std::move(value);
        }

        template <>
        inline Value Value::Create<ValueType::kDeviceIDType, identifier_t>(const identifier_t& deviceID)
        {
            Value value = Value(ValueType::kDeviceIDType);
            value.SetDeviceID(deviceID);
            return std::move(value);
        }

        template <>
        inline Value Value::Create<ValueType::kServiceIDType, identifier_t>(const identifier_t& serviceID)
        {
            Value value = Value(ValueType::kServiceIDType);
            value.SetServiceID(serviceID);
            return std::move(value);
        }
    }
}