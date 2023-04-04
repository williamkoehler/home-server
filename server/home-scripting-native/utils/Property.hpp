#pragma once
#include "../common.hpp"
#include <home-scripting/utils/Value.hpp>

namespace server
{
    namespace scripting
    {
        namespace native
        {
            class NativeScriptImpl;

            template <typename P = void*, class T = NativeScriptImpl>
            using PropertyGetterDefinition = P (T::*)() const;

            template <typename P = void*, class T = NativeScriptImpl>
            using PropertySetterDefinition = void (T::*)(const P&);

            template <typename P, class T>
            class PropertyCallbackImpl;

            template <typename P, class T>
            class PropertyReferenceImpl;

            class Property
            {
              public:
                Property()
                {
                }
                virtual ~Property()
                {
                }

                template <typename P, class T>
                static UniqueRef<Property> Create(PropertyGetterDefinition<P, T> getter,
                                                  PropertySetterDefinition<P, T> setter = nullptr)
                {
                    assert(getter != nullptr);
                    return boost::make_unique<PropertyCallbackImpl<P, T>>(getter, setter);
                }
                template <typename P, class T>
                static UniqueRef<Property> Create(P T::*property)
                {
                    assert(property != nullptr);
                    return boost::make_unique<PropertyReferenceImpl<P, T>>(property);
                }

                virtual ValueType GetType() = 0;

                virtual Value Get(void* self) = 0;
                virtual void Set(void* self, const Value& value) = 0;
            };

#define PROPERTY_GETTER (static_cast<T*>(self)->*getter)
#define PROPERTY_SETTER (static_cast<T*>(self)->*setter)
#define PROPERTY_CALLBACK_IMPLEMENTATION(type, valueType, getterExpr, setterCondition, setterExpr)                     \
    template <class T>                                                                                                 \
    class PropertyCallbackImpl<type, T> final : public Property                                                              \
    {                                                                                                                  \
      private:                                                                                                         \
        PropertyGetterDefinition<type, T> getter;                                                                        \
        PropertySetterDefinition<type, T> setter;                                                                        \
                                                                                                                       \
      public:                                                                                                          \
        PropertyCallbackImpl<type, T>(PropertyGetterDefinition<type, T> getter, PropertySetterDefinition<type, T> setter)  \
            : getter(getter), setter(setter)                                                                           \
        {                                                                                                              \
            assert(getter != nullptr);                                                                                 \
        }                                                                                                              \
        virtual ValueType GetType() override                                                                           \
        {                                                                                                              \
            return valueType;                                                                                          \
        }                                                                                                              \
        virtual Value Get(void* self) override                                                                         \
        {                                                                                                              \
            return getterExpr;                                                                                         \
        }                                                                                                              \
        virtual void Set(void* self, const Value& value) override                                                      \
        {                                                                                                              \
            if (setterCondition && setter != nullptr)                                                                                       \
                setterExpr;                                                                                            \
        }                                                                                                              \
    };

            PROPERTY_CALLBACK_IMPLEMENTATION(bool, ValueType::kBooleanType, Value(PROPERTY_GETTER()), value.IsBoolean(),
                                             PROPERTY_SETTER(value.GetBoolean()))
            PROPERTY_CALLBACK_IMPLEMENTATION(double_t, ValueType::kNumberType, Value(PROPERTY_GETTER()),
                                             value.IsNumber(), PROPERTY_SETTER(value.GetNumber()))
            PROPERTY_CALLBACK_IMPLEMENTATION(size_t, ValueType::kNumberType, Value((const double_t&)PROPERTY_GETTER),
                                             value.IsNumber(), PROPERTY_SETTER((size_t)value.GetNumber()))
            PROPERTY_CALLBACK_IMPLEMENTATION(int8_t, ValueType::kNumberType, Value((const int8_t&)PROPERTY_GETTER()),
                                             value.IsNumber(), PROPERTY_SETTER((int8_t)value.GetNumber()))
            PROPERTY_CALLBACK_IMPLEMENTATION(uint8_t, ValueType::kNumberType, Value((const uint8_t&)PROPERTY_GETTER()),
                                             value.IsNumber(), PROPERTY_SETTER((uint8_t)value.GetNumber()))
            PROPERTY_CALLBACK_IMPLEMENTATION(int16_t, ValueType::kNumberType, Value((const int16_t&)PROPERTY_GETTER()),
                                             value.IsNumber(), PROPERTY_SETTER((int16_t)value.GetNumber()))
            PROPERTY_CALLBACK_IMPLEMENTATION(uint16_t, ValueType::kNumberType,
                                             Value((const uint16_t&)PROPERTY_GETTER()), value.IsNumber(),
                                             PROPERTY_SETTER((uint16_t)value.GetNumber()))
            PROPERTY_CALLBACK_IMPLEMENTATION(int32_t, ValueType::kNumberType, Value((const int32_t&)PROPERTY_GETTER()),
                                             value.IsNumber(), PROPERTY_SETTER((int32_t)value.GetNumber()))
            PROPERTY_CALLBACK_IMPLEMENTATION(uint32_t, ValueType::kNumberType,
                                             Value((const uint32_t&)PROPERTY_GETTER()), value.IsNumber(),
                                             PROPERTY_SETTER((uint32_t)value.GetNumber()))
            PROPERTY_CALLBACK_IMPLEMENTATION(std::string, ValueType::kStringType, Value(PROPERTY_GETTER()),
                                             value.IsString(), PROPERTY_SETTER(value.GetString()))
            PROPERTY_CALLBACK_IMPLEMENTATION(Endpoint, ValueType::kEndpointType, Value(PROPERTY_GETTER()),
                                             value.IsEndpoint(), PROPERTY_SETTER(value.GetEndpoint()))
            PROPERTY_CALLBACK_IMPLEMENTATION(Color, ValueType::kColorType, Value(PROPERTY_GETTER()), value.IsColor(),
                                             PROPERTY_SETTER(value.GetColor()))

#undef PROPERTY_GETTER
#undef PROPERTY_SETTER
#undef PROPERTY_CALLBACK_IMPLEMENTATION

#define PROPERTY_REFERENCE (static_cast<T*>(self)->*reference)
#define PROPERTY_REFERENCE_IMPLEMENTATION(type, valueType, getterExpr, setterCondition, setterExpr)                    \
    template <class T>                                                                                                 \
    class PropertyReferenceImpl<type, T> : public Property                                                             \
    {                                                                                                                  \
      private:                                                                                                         \
        type T::*reference;                                                                                            \
                                                                                                                       \
      public:                                                                                                          \
        PropertyReferenceImpl<type, T>(type T::*reference) : reference(reference)                                      \
        {                                                                                                              \
            assert(reference != nullptr);                                                                              \
        }                                                                                                              \
        virtual ValueType GetType() override                                                                           \
        {                                                                                                              \
            return valueType;                                                                                          \
        }                                                                                                              \
        virtual Value Get(void* self) override                                                                         \
        {                                                                                                              \
            return getterExpr;                                                                                         \
        }                                                                                                              \
        virtual void Set(void* self, const Value& value) override                                                      \
        {                                                                                                              \
            if (setterCondition)                                                                                       \
                setterExpr;                                                                                            \
        }                                                                                                              \
    };

            PROPERTY_REFERENCE_IMPLEMENTATION(bool, ValueType::kBooleanType, Value(PROPERTY_REFERENCE),
                                              value.IsBoolean(), PROPERTY_REFERENCE = value.GetBoolean())
            PROPERTY_REFERENCE_IMPLEMENTATION(double_t, ValueType::kNumberType, Value(PROPERTY_REFERENCE),
                                              value.IsNumber(), PROPERTY_REFERENCE = value.GetNumber())
            PROPERTY_REFERENCE_IMPLEMENTATION(size_t, ValueType::kNumberType,
                                              Value((const double_t&)PROPERTY_REFERENCE), value.IsNumber(),
                                              PROPERTY_REFERENCE = (size_t)value.GetNumber())
            PROPERTY_REFERENCE_IMPLEMENTATION(int8_t, ValueType::kNumberType, Value((const int8_t&)PROPERTY_REFERENCE),
                                              value.IsNumber(), PROPERTY_REFERENCE = (int8_t)value.GetNumber())
            PROPERTY_REFERENCE_IMPLEMENTATION(uint8_t, ValueType::kNumberType,
                                              Value((const uint8_t&)PROPERTY_REFERENCE), value.IsNumber(),
                                              PROPERTY_REFERENCE = (uint8_t)value.GetNumber())
            PROPERTY_REFERENCE_IMPLEMENTATION(int16_t, ValueType::kNumberType,
                                              Value((const int16_t&)PROPERTY_REFERENCE), value.IsNumber(),
                                              PROPERTY_REFERENCE = (int16_t)value.GetNumber())
            PROPERTY_REFERENCE_IMPLEMENTATION(uint16_t, ValueType::kNumberType,
                                              Value((const uint16_t&)PROPERTY_REFERENCE), value.IsNumber(),
                                              PROPERTY_REFERENCE = (uint16_t)value.GetNumber())
            PROPERTY_REFERENCE_IMPLEMENTATION(int32_t, ValueType::kNumberType,
                                              Value((const int32_t&)PROPERTY_REFERENCE), value.IsNumber(),
                                              PROPERTY_REFERENCE = (int32_t)value.GetNumber())
            PROPERTY_REFERENCE_IMPLEMENTATION(uint32_t, ValueType::kNumberType,
                                              Value((const uint32_t&)PROPERTY_REFERENCE), value.IsNumber(),
                                              PROPERTY_REFERENCE = (uint32_t)value.GetNumber())
            PROPERTY_REFERENCE_IMPLEMENTATION(std::string, ValueType::kStringType, Value(PROPERTY_REFERENCE),
                                              value.IsString(), PROPERTY_REFERENCE = value.GetString())
            PROPERTY_REFERENCE_IMPLEMENTATION(Endpoint, ValueType::kEndpointType, Value(PROPERTY_REFERENCE),
                                              value.IsEndpoint(), PROPERTY_REFERENCE = value.GetEndpoint())
            PROPERTY_REFERENCE_IMPLEMENTATION(Color, ValueType::kColorType, Value(PROPERTY_REFERENCE), value.IsColor(),
                                              PROPERTY_REFERENCE = value.GetColor())

#undef PROPERTY_REFERENCE
#undef PROPERTY_REFERENCE_IMPLEMENTATION
        }
    }
}