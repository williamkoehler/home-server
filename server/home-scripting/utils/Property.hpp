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

        enum PropertyType : uint8_t
        {
            kUnknownType,
            kBooleanType,
            kIntegerType,
            kNumberType,
            kStringType,
            kEndpointType,
            kColorType,
        };

        std::string StringifyPropertyType(PropertyType type);
        PropertyType ParsePropertyType(const std::string& type);

        /// @brief Thread safe property that can be read and written to from mutliple threads. Uses atomic operation and
        /// spinlocks
        class Property
        {
          public:
            static Ref<Property> Create(PropertyType type);

            virtual PropertyType GetType() const = 0;

            inline bool IsBoolean() const
            {
                return GetType() == PropertyType::kBooleanType;
            }
            inline bool IsInteger() const
            {
                return GetType() == PropertyType::kIntegerType;
            }
            inline bool IsNumber() const
            {
                return GetType() == PropertyType::kNumberType;
            }
            inline bool IsString() const
            {
                return GetType() == PropertyType::kStringType;
            }
            inline bool IsEndpoint() const
            {
                return GetType() == PropertyType::kEndpointType;
            }
            inline bool IsColor() const
            {
                return GetType() == PropertyType::kColorType;
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
            virtual bool JsonSet(rapidjson::Value& input, rapidjson::Document::AllocatorType& allocator)
            {
                return false;
            }
        };

        class NullProperty : public Property
        {
          public:
            static Ref<NullProperty> Create();

            virtual PropertyType GetType() const override;
        };

        class BooleanProperty : public Property
        {
          private:
            bool value;

          public:
            static Ref<BooleanProperty> Create();

            virtual PropertyType GetType() const override;
            virtual bool GetBoolean() override;
            virtual void SetBoolean(bool v) override;

            virtual rapidjson::Value JsonGet(rapidjson::Document::AllocatorType& allocator) override;
            virtual bool JsonSet(rapidjson::Value& input, rapidjson::Document::AllocatorType& allocator) override;
        };

        class IntegerProperty : public Property
        {
          private:
            int64_t value;

          public:
            static Ref<IntegerProperty> Create();

            virtual PropertyType GetType() const override;
            virtual int64_t GetInteger() override;
            virtual void SetInteger(int64_t v) override;

            virtual double GetNumber() override;
            virtual void SetNumber(double v) override;

            virtual rapidjson::Value JsonGet(rapidjson::Document::AllocatorType& allocator) override;
            virtual bool JsonSet(rapidjson::Value& input, rapidjson::Document::AllocatorType& allocator) override;
        };

        class NumberProperty : public Property
        {
          private:
            double value;

          public:
            static Ref<NumberProperty> Create();

            virtual PropertyType GetType() const override;
            virtual int64_t GetInteger() override;
            virtual void SetInteger(int64_t v) override;

            virtual double GetNumber() override;
            virtual void SetNumber(double v) override;

            virtual rapidjson::Value JsonGet(rapidjson::Document::AllocatorType& allocator) override;
            virtual bool JsonSet(rapidjson::Value& input, rapidjson::Document::AllocatorType& allocator) override;
        };

        class StringProperty : public Property
        {
          private:
            std::string value;

          public:
            static Ref<StringProperty> Create();

            virtual PropertyType GetType() const override;
            virtual std::string GetString() override;
            virtual void SetString(const std::string& v) override;

            virtual rapidjson::Value JsonGet(rapidjson::Document::AllocatorType& allocator) override;
            virtual bool JsonSet(rapidjson::Value& input, rapidjson::Document::AllocatorType& allocator) override;
        };

        class EndpointProperty : public Property
        {
          private:
            Endpoint value;

          public:
            static Ref<EndpointProperty> Create();

            virtual PropertyType GetType() const override;
            virtual Endpoint GetEndpoint() override;
            virtual void SetEndpoint(const Endpoint& v) override;

            virtual rapidjson::Value JsonGet(rapidjson::Document::AllocatorType& allocator) override;
            virtual bool JsonSet(rapidjson::Value& input, rapidjson::Document::AllocatorType& allocator) override;
        };

        class ColorProperty : public Property
        {
          private:
            Color value;

          public:
            static Ref<ColorProperty> Create();

            virtual PropertyType GetType() const override;
            virtual Color GetColor() override;
            virtual void SetColor(const Color& v) override;

            virtual rapidjson::Value JsonGet(rapidjson::Document::AllocatorType& allocator) override;
            virtual bool JsonSet(rapidjson::Value& input, rapidjson::Document::AllocatorType& allocator) override;
        };
    }
}