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
            kUnknownType = 0,
            kNullType,
            kBooleanType,
            kIntegerType,
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

            std::array<uint8_t, static_max<bool, ssize_t, double_t, std::string, Endpoint, Color, identifier_t>::value>
                value;

          public:
            explicit Value();
            explicit Value(bool boolean);
            explicit Value(ssize_t integer);
            explicit Value(double_t number);
            explicit Value(const std::string& string);
            explicit Value(const std::string_view& string);
            explicit Value(const Endpoint& endpoint);
            explicit Value(const Color& color);
            explicit Value(ValueType type);
            Value(const Value& other);
            Value(Value&& other) noexcept;
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
            static Value Create(const rapidjson::Value& json);

            void operator=(const Value& other) noexcept;
            void operator=(Value&& other) noexcept;

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
                return type == ValueType::kIntegerType || type == ValueType::kNumberType;
            }
            inline bool IsRealInteger() const
            {
                return type == ValueType::kIntegerType;
            }
            inline bool IsNumber() const
            {
                return type == ValueType::kNumberType || type == ValueType::kIntegerType;
            }
            inline bool IsRealNumber() const
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
                return *(bool*)value.data();
            }
            inline const bool& GetBoolean() const
            {
                assert(type == ValueType::kBooleanType);
                return *(bool*)value.data();
            }

            inline int64_t GetInteger() const
            {
                assert(type == ValueType::kIntegerType || type == ValueType::kNumberType);
                if (type == ValueType::kNumberType)
                    return (ssize_t)(*(double_t*)value.data());
                else
                    return *(ssize_t*)value.data();
            }

            inline ssize_t& GetRealInteger()
            {
                assert(type == ValueType::kIntegerType);
                return *(ssize_t*)value.data();
            }
            inline const ssize_t& GetRealInteger() const
            {
                assert(type == ValueType::kIntegerType);
                return *(ssize_t*)value.data();
            }

            inline double_t GetNumber() const
            {
                assert(type == ValueType::kNumberType || type == ValueType::kIntegerType);
                if (type == ValueType::kIntegerType)
                    return (double_t)(*(ssize_t*)value.data());
                else
                    return *(double_t*)value.data();
            }

            inline double_t& GetRealNumber()
            {
                assert(type == ValueType::kNumberType);
                return *(double_t*)value.data();
            }
            inline const double_t& GetRealNumber() const
            {
                assert(type == ValueType::kNumberType);
                return *(double_t*)value.data();
            }

            inline std::string& GetString()
            {
                assert(type == ValueType::kStringType);
                return *(std::string*)value.data();
            }
            inline const std::string& GetString() const
            {
                assert(type == ValueType::kStringType);
                return *(std::string*)value.data();
            }

            inline Endpoint& GetEndpoint()
            {
                assert(type == ValueType::kEndpointType);
                return *(Endpoint*)value.data();
            }
            inline const Endpoint& GetEndpoint() const
            {
                assert(type == ValueType::kEndpointType);
                return *(Endpoint*)value.data();
            }

            inline Color& GetColor()
            {
                assert(type == ValueType::kColorType);
                return *(Color*)value.data();
            }
            inline const Color& GetColor() const
            {
                assert(type == ValueType::kColorType);
                return *(Color*)value.data();
            }

            inline identifier_t& GetRoomID()
            {
                assert(type == ValueType::kRoomIDType);
                return *(identifier_t*)value.data();
            }
            inline const identifier_t& GetRoomID() const
            {
                assert(type == ValueType::kRoomIDType);
                return *(identifier_t*)value.data();
            }

            inline identifier_t& GetDeviceID()
            {
                assert(type == ValueType::kDeviceIDType);
                return *(identifier_t*)value.data();
            }
            inline const identifier_t& GetDeviceID() const
            {
                assert(type == ValueType::kDeviceIDType);
                return *(identifier_t*)value.data();
            }

            inline identifier_t& GetServiceID()
            {
                assert(type == ValueType::kServiceIDType);
                return *(identifier_t*)value.data();
            }
            inline const identifier_t& GetServiceID() const
            {
                assert(type == ValueType::kServiceIDType);
                return *(identifier_t*)value.data();
            }

            inline void SetBoolean(bool v)
            {
                assert(type == ValueType::kBooleanType);
                *(bool*)value.data() = v;
            }
            inline void SetInteger(ssize_t v)
            {
                assert(type == ValueType::kIntegerType || type == ValueType::kNumberType);
                if (type == kNumberType)
                    *(double_t*)value.data() = (double_t)v;
                else
                    *(ssize_t*)value.data() = v;
            }
            inline void SetNumber(double_t v)
            {
                assert(type == ValueType::kNumberType || type == ValueType::kIntegerType);
                if (type == kIntegerType)
                    *(ssize_t*)value.data() = (ssize_t)v;
                else
                    *(double_t*)value.data() = v;
            }
            inline void SetString(const std::string& v)
            {
                assert(type == ValueType::kStringType);
                *(std::string*)value.data() = v;
            }
            inline void SetStringView(const std::string_view& v)
            {
                assert(type == ValueType::kStringType);
                *(std::string*)value.data() = v;
            }
            inline void SetEndpoint(const Endpoint& v)
            {
                assert(type == ValueType::kEndpointType);

                Endpoint& endpoint = *(Endpoint*)value.data();
                endpoint.host = v.host;
                endpoint.port = v.port;
            }
            inline void SetColor(const Color& v)
            {
                assert(type == ValueType::kColorType);

                Color& color = *(Color*)value.data();
                color.red = v.red;
                color.green = v.green;
                color.blue = v.blue;
            }
            inline void SetRoomID(const identifier_t& v)
            {
                assert(type == ValueType::kRoomIDType);
                *(identifier_t*)value.data() = v;
            }
            inline void SetDeviceID(const identifier_t& v)
            {
                assert(type == ValueType::kDeviceIDType);
                *(identifier_t*)value.data() = v;
            }
            inline void SetServiceID(const identifier_t& v)
            {
                assert(type == ValueType::kServiceIDType);
                *(identifier_t*)value.data() = v;
            }

            void Assign(const Value& other);

            std::string ToString();

            rapidjson::Value JsonGet(rapidjson::Document::AllocatorType& allocator) const;
            bool JsonSet(const rapidjson::Value& input);
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
            return value;
        }

        template <>
        inline Value Value::Create<ValueType::kDeviceIDType, identifier_t>(const identifier_t& deviceID)
        {
            Value value = Value(ValueType::kDeviceIDType);
            value.SetDeviceID(deviceID);
            return value;
        }

        template <>
        inline Value Value::Create<ValueType::kServiceIDType, identifier_t>(const identifier_t& serviceID)
        {
            Value value = Value(ValueType::kServiceIDType);
            value.SetServiceID(serviceID);
            return value;
        }
    }
}