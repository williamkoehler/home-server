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
            kBooleanType,
            kIntegerType,
            kNumberType,
            kStringType,
            kEndpointType,
            kColorType,
        };

        std::string StringifyValueType(ValueType type);
        ValueType ParseValueType(const std::string& type);

        class Value
        {
          public:
            static Ref<Value> Create(ValueType type);

            virtual ValueType GetType() const = 0;

            inline bool IsBoolean() const
            {
                return GetType() == ValueType::kBooleanType;
            }
            inline bool IsInteger() const
            {
                return GetType() == ValueType::kIntegerType;
            }
            inline bool IsNumber() const
            {
                return GetType() == ValueType::kNumberType;
            }
            inline bool IsString() const
            {
                return GetType() == ValueType::kStringType;
            }
            inline bool IsEndpoint() const
            {
                return GetType() == ValueType::kEndpointType;
            }
            inline bool IsColor() const
            {
                return GetType() == ValueType::kColorType;
            }

            virtual bool GetBoolean()
            {
                return false;
            }
            virtual int64_t GetInteger()
            {
                return 0;
            }
            virtual double GetNumber()
            {
                return 0.0;
            }
            virtual std::string GetString()
            {
                return "";
            }
            virtual Endpoint GetEndpoint()
            {
                return {"", 0};
            }
            virtual Color GetColor()
            {
                return {0, 0, 0};
            }

            virtual void SetBoolean(bool value)
            {
            }
            virtual void SetInteger(int64_t value)
            {
            }
            virtual void SetNumber(double value)
            {
            }
            virtual void SetString(const std::string& value)
            {
            }
            virtual void SetEndpoint(const Endpoint& endpoint)
            {
            }
            virtual void SetColor(const Color& color)
            {
            }

            virtual rapidjson::Value JsonGet(rapidjson::Document::AllocatorType& allocator)
            {
                return rapidjson::Value(rapidjson::kNullType);
            }
            virtual bool JsonSet(rapidjson::Value& input)
            {
                return false;
            }
        };

        class NullValue : public Value
        {
          public:
            static Ref<NullValue> Create();

            virtual ValueType GetType() const override;
        };

        class BooleanValue : public Value
        {
          private:
            bool value;

          public:
            static Ref<BooleanValue> Create();

            virtual ValueType GetType() const override;
            virtual bool GetBoolean() override;
            virtual void SetBoolean(bool v) override;

            virtual rapidjson::Value JsonGet(rapidjson::Document::AllocatorType& allocator) override;
            virtual bool JsonSet(rapidjson::Value& input) override;
        };

        class IntegerValue : public Value
        {
          private:
            int64_t value;

          public:
            static Ref<IntegerValue> Create();

            virtual ValueType GetType() const override;
            virtual int64_t GetInteger() override;
            virtual void SetInteger(int64_t v) override;

            virtual double GetNumber() override;
            virtual void SetNumber(double v) override;

            virtual rapidjson::Value JsonGet(rapidjson::Document::AllocatorType& allocator) override;
            virtual bool JsonSet(rapidjson::Value& input) override;
        };

        class NumberValue : public Value
        {
          private:
            double value;

          public:
            static Ref<NumberValue> Create();

            virtual ValueType GetType() const override;
            virtual int64_t GetInteger() override;
            virtual void SetInteger(int64_t v) override;

            virtual double GetNumber() override;
            virtual void SetNumber(double v) override;

            virtual rapidjson::Value JsonGet(rapidjson::Document::AllocatorType& allocator) override;
            virtual bool JsonSet(rapidjson::Value& input) override;
        };

        class StringValue : public Value
        {
          private:
            std::string value;

          public:
            static Ref<StringValue> Create();

            virtual ValueType GetType() const override;
            virtual std::string GetString() override;
            virtual void SetString(const std::string& v) override;

            virtual rapidjson::Value JsonGet(rapidjson::Document::AllocatorType& allocator) override;
            virtual bool JsonSet(rapidjson::Value& input) override;
        };

        class EndpointValue : public Value
        {
          private:
            Endpoint value;

          public:
            static Ref<EndpointValue> Create();

            virtual ValueType GetType() const override;
            virtual Endpoint GetEndpoint() override;
            virtual void SetEndpoint(const Endpoint& v) override;

            virtual rapidjson::Value JsonGet(rapidjson::Document::AllocatorType& allocator) override;
            virtual bool JsonSet(rapidjson::Value& input) override;
        };

        class ColorValue : public Value
        {
          private:
            Color value;

          public:
            static Ref<ColorValue> Create();

            virtual ValueType GetType() const override;
            virtual Color GetColor() override;
            virtual void SetColor(const Color& v) override;

            virtual rapidjson::Value JsonGet(rapidjson::Document::AllocatorType& allocator) override;
            virtual bool JsonSet(rapidjson::Value& input) override;
        };
    }
}