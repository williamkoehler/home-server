#pragma once
#include "../common.hpp"
#include <scripting/property_flags.hpp>
#include <scripting/value.hpp>

namespace server
{
    namespace scripting
    {
        namespace native
        {
            class NativeScriptImplementation;

            template <class T = NativeScriptImplementation>
            using PropertyUpdateDefinition = void (T::*)();

            template <typename P, class T>
            class PropertyReferenceImpl;
            template <typename P, class T>
            class PropertyReadonlyReferenceImpl;

            class Property
            {
              private:
                uint8_t flags;

              public:
                Property(uint8_t propertyFlags) : flags(propertyFlags)
                {
                }
                virtual ~Property()
                {
                }

                /// @brief Create property
                ///
                /// @tparam P Property value type
                /// @tparam T Script type
                /// @param property Property reference
                /// @param propertyFlags Property flags
                /// @return UniqueRef<Property> Property
                template <typename P, class T>
                static inline UniqueRef<Property> Create(P T::*property, PropertyUpdateDefinition<T> updateCallback,
                                                         PropertyFlags propertyFlags = kPropertyFlag_Visible)
                {
                    assert(property != nullptr);
                    return boost::make_unique<PropertyReferenceImpl<P, T>>(property, updateCallback, propertyFlags);
                }

                /// @brief Create property
                ///
                /// @tparam P Property value type
                /// @tparam T Script type
                /// @param property Property reference
                /// @param propertyFlags Property flags
                /// @return UniqueRef<Property> Property
                template <typename P, class T>
                static inline UniqueRef<Property> Create(P T::*property,
                                                         PropertyFlags propertyFlags = kPropertyFlag_Visible)
                {
                    return Create<P, T>(property, nullptr, propertyFlags);
                }

                /// @brief Create readonly property
                ///
                /// @tparam P Property value type
                /// @tparam T Script type
                /// @param property Property reference
                /// @param propertyFlags Property flags
                /// @return UniqueRef<Property> Property
                template <typename P, class T>
                static UniqueRef<Property> CreateReadonly(P T::*property,
                                                          PropertyFlags propertyFlags = kPropertyFlag_Visible)
                {
                    assert(property != nullptr);
                    return boost::make_unique<PropertyReadonlyReferenceImpl<P, T>>(property, propertyFlags);
                }

                /// @brief Get property value type
                ///
                /// @return ValueType Property value type
                virtual ValueType GetType() const = 0;

                /// @brief Get property flags
                ///
                /// @return uint8_t
                inline uint8_t GetFlags() const
                {
                    return flags;
                }

                virtual Value Get(void* self) const = 0;
                virtual void Set(void* self, const Value& value) = 0;
            };

#define PROPERTY_REFERENCE (static_cast<T*>(self)->*reference)

#define PROPERTY_REFERENCE_IMPLEMENTATION(type, valueType, getterExpr, setterCondition, setterExpr)                    \
    template <class T>                                                                                                 \
    class PropertyReferenceImpl<type, T> : public Property                                                             \
    {                                                                                                                  \
      private:                                                                                                         \
        type T::*reference;                                                                                            \
        PropertyUpdateDefinition<T> updateMethod;                                                                      \
                                                                                                                       \
      public:                                                                                                          \
        PropertyReferenceImpl<type, T>(type T::*reference, PropertyUpdateDefinition<T> updateMethod,                   \
                                       uint8_t propertyFlags)                                                          \
            : Property(propertyFlags), reference(reference), updateMethod(updateMethod)                                \
        {                                                                                                              \
            assert(reference != nullptr);                                                                              \
        }                                                                                                              \
        virtual ValueType GetType() const override                                                                     \
        {                                                                                                              \
            return valueType;                                                                                          \
        }                                                                                                              \
        virtual Value Get(void* self) const override                                                                   \
        {                                                                                                              \
            return getterExpr;                                                                                         \
        }                                                                                                              \
        virtual void Set(void* self, const Value& value) override                                                      \
        {                                                                                                              \
            if (setterCondition)                                                                                       \
            {                                                                                                          \
                setterExpr;                                                                                            \
                if (updateMethod != nullptr)                                                                           \
                    (static_cast<T*>(self)->*updateMethod)();                                                          \
            }                                                                                                          \
        }                                                                                                              \
    };                                                                                                                 \
                                                                                                                       \
    template <class T>                                                                                                 \
    class PropertyReadonlyReferenceImpl<type, T> : public Property                                                     \
    {                                                                                                                  \
      private:                                                                                                         \
        type T::*reference;                                                                                            \
                                                                                                                       \
      public:                                                                                                          \
        PropertyReadonlyReferenceImpl<type, T>(type T::*reference, uint8_t propertyFlags)                              \
            : Property(propertyFlags), reference(reference)                                                            \
        {                                                                                                              \
            assert(reference != nullptr);                                                                              \
        }                                                                                                              \
        virtual ValueType GetType() const override                                                                     \
        {                                                                                                              \
            return valueType;                                                                                          \
        }                                                                                                              \
        virtual Value Get(void* self) const override                                                                   \
        {                                                                                                              \
            return getterExpr;                                                                                         \
        }                                                                                                              \
        virtual void Set(void* self, const Value& value) override                                                      \
        {                                                                                                              \
            (void)self;                                                                                                \
            (void)value;                                                                                               \
        }                                                                                                              \
    };

            PROPERTY_REFERENCE_IMPLEMENTATION(bool, ValueType::kBooleanType, Value(PROPERTY_REFERENCE),
                                              value.IsBoolean(), PROPERTY_REFERENCE = value.GetBoolean())
            PROPERTY_REFERENCE_IMPLEMENTATION(double_t, ValueType::kNumberType, Value(PROPERTY_REFERENCE),
                                              value.IsNumber(), PROPERTY_REFERENCE = value.GetNumber())
            PROPERTY_REFERENCE_IMPLEMENTATION(ssize_t, ValueType::kIntegerType, Value(PROPERTY_REFERENCE),
                                              value.IsInteger(), PROPERTY_REFERENCE = value.GetInteger())
            PROPERTY_REFERENCE_IMPLEMENTATION(size_t, ValueType::kIntegerType, Value((ssize_t)PROPERTY_REFERENCE),
                                              value.IsInteger(), PROPERTY_REFERENCE = (size_t)value.GetInteger())
            PROPERTY_REFERENCE_IMPLEMENTATION(int8_t, ValueType::kIntegerType, Value((ssize_t)PROPERTY_REFERENCE),
                                              value.IsInteger(), PROPERTY_REFERENCE = (int8_t)value.GetInteger())
            PROPERTY_REFERENCE_IMPLEMENTATION(uint8_t, ValueType::kIntegerType, Value((ssize_t)PROPERTY_REFERENCE),
                                              value.IsInteger(), PROPERTY_REFERENCE = (uint8_t)value.GetInteger())
            PROPERTY_REFERENCE_IMPLEMENTATION(int16_t, ValueType::kIntegerType, Value((ssize_t)PROPERTY_REFERENCE),
                                              value.IsInteger(), PROPERTY_REFERENCE = (int16_t)value.GetInteger())
            PROPERTY_REFERENCE_IMPLEMENTATION(uint16_t, ValueType::kIntegerType, Value((ssize_t)PROPERTY_REFERENCE),
                                              value.IsInteger(), PROPERTY_REFERENCE = (uint16_t)value.GetInteger())
            PROPERTY_REFERENCE_IMPLEMENTATION(int32_t, ValueType::kIntegerType, Value((ssize_t)PROPERTY_REFERENCE),
                                              value.IsInteger(), PROPERTY_REFERENCE = (int32_t)value.GetInteger())
            PROPERTY_REFERENCE_IMPLEMENTATION(uint32_t, ValueType::kIntegerType, Value((ssize_t)PROPERTY_REFERENCE),
                                              value.IsInteger(), PROPERTY_REFERENCE = (uint32_t)value.GetInteger())
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