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
            using PropertyGetterCallback = P (T::*)() const;
            template <typename P = void*, class T = NativeScriptImpl>
            using PropertyGetterFunctionCallback = P (*)(T*);

            template <typename P, class T = NativeScriptImpl>
            union PropertyGetterCallbackConversion
            {
                PropertyGetterCallback<P, T> f1;
                PropertyGetterFunctionCallback<P, T> f2;
                void* f3;
            };

            template <typename P = void*, class T = NativeScriptImpl>
            using PropertySetterCallback = void (T::*)(const P&);
            template <typename P = void*, class T = NativeScriptImpl>
            using PropertySetterFunctionCallback = void (*)(T*, const P&);

            template <typename P, class T = NativeScriptImpl>
            union PropertySetterCallbackConversion
            {
                PropertySetterCallback<P, T> f1;
                PropertySetterFunctionCallback<P, T> f2;
                void* f3;
            };

            class Property
            {
              private:
                ValueType type;
                void* getter;
                void* setter;
                void* update;
                size_t updateInterval;
                size_t lastUpdateTime;

                template <typename P>
                static inline Property CreateImpl(void* getter, void* setter);

              public:
                Property();
                Property(ValueType type, void* getter, void* setter);
                ~Property();
                template <class T, typename P>
                static inline Property Create(PropertyGetterCallback<P, T> getter,
                                              PropertySetterCallback<P, T> setter = nullptr);

                /// @brief Get property type
                ///
                /// @return ValueType Value type
                inline ValueType GetType() const
                {
                    return type;
                }

                /// @brief Get getter function pointer
                ///
                /// @return void* Function pointer
                inline void* GetGetterFunction() const
                {
                    return getter;
                }

                /// @brief Get getter function pointer
                ///
                /// @tparam P Property Type
                /// @tparam T Base class
                /// @return PropertyGetterCallback<P, T> Function pointer
                template <typename P, class T = NativeScriptImpl>
                inline PropertyGetterFunctionCallback<P, T> GetGetterFunction() const
                {
                    return PropertyGetterCallbackConversion<P, T>{.f3 = getter}.f2;
                }

                /// @brief Get setter function pointer
                ///
                /// @return void* Function pointer
                inline void* GetSetterFunction() const
                {
                    return setter;
                }

                /// @brief Get setter function pointer
                ///
                /// @tparam P Property Type
                /// @tparam T Base class
                /// @return PropertySetterCallback<P, T> Function pointer
                template <typename P, class T = NativeScriptImpl>
                inline PropertySetterFunctionCallback<P, T> GetSetterFunction() const
                {
                    return PropertySetterCallbackConversion<P, T>{.f3 = setter}.f2;
                }
            };

            template <>
            inline Property Property::CreateImpl<bool>(void* getter, void* setter)
            {
                return Property(ValueType::kBooleanType, getter, setter);
            }

            template <>
            inline Property Property::CreateImpl<double>(void* getter, void* setter)
            {
                return Property(ValueType::kNumberType, getter, setter);
            }

            template <>
            inline Property Property::CreateImpl<std::string>(void* getter, void* setter)
            {
                return Property(ValueType::kStringType, getter, setter);
            }

            template <>
            inline Property Property::CreateImpl<Endpoint>(void* getter, void* setter)
            {
                return Property(ValueType::kEndpointType, getter, setter);
            }

            template <>
            inline Property Property::CreateImpl<Color>(void* getter, void* setter)
            {
                return Property(ValueType::kColorType, getter, setter);
            }

            template <class T, typename P>
            inline Property Property::Create(PropertyGetterCallback<P, T> getter, PropertySetterCallback<P, T> setter)
            {
                return Property::CreateImpl<P>(PropertyGetterCallbackConversion<P, T>{.f1 = getter}.f3,
                                               PropertySetterCallbackConversion<P, T>{.f1 = setter}.f3);
            }
        }
    }
}