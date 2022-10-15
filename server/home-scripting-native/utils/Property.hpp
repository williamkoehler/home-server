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

            template <typename P, class T = NativeScriptImpl>
            union PropertyGetterCallbackConversion
            {
                PropertyGetterCallback<P, T> f1;
                PropertyGetterCallback<P> f2;
                PropertyGetterCallback<> f3;
                void* f4;
            };

            template <typename P = void*, class T = NativeScriptImpl>
            using PropertySetterCallback = void (T::*)(const P&);

            template <typename P, class T = NativeScriptImpl>
            union PropertySetterCallbackConversion
            {
                PropertySetterCallback<P, T> f1;
                PropertySetterCallback<P> f2;
                PropertySetterCallback<> f3;
                void* f4;
            };

            class Property
            {
              private:
                ValueType type;
                void* getter;
                void* setter;

                template <typename P>
                static inline Property CreateImpl(PropertyGetterCallback<P> getter, PropertySetterCallback<P> setter);

              public:
                Property();
                Property(ValueType type, void* getter, void* setter);
                ~Property();
                template <class T, typename P>
                static inline Property Create(PropertyGetterCallback<P, T> getter,
                                              PropertySetterCallback<P, T> setter = nullptr);

                inline ValueType GetType() const
                {
                    return type;
                }

                inline void* GetGetter() const
                {
                    return getter;
                }
                template <typename P, class T = NativeScriptImpl>
                inline PropertyGetterCallback<P, T> GetGetter() const
                {
                    return PropertyGetterCallbackConversion<P, T>{.f4 = getter}.f1;
                }

                inline void* GetSetter() const
                {
                    return setter;
                }
                template <typename P, class T = NativeScriptImpl>
                inline PropertySetterCallback<P, T> GetSetter() const
                {
                    return PropertySetterCallbackConversion<P, T>{.f4 = setter}.f1;
                }
            };

            template <>
            inline Property Property::CreateImpl<bool>(PropertyGetterCallback<bool> getter,
                                                       PropertySetterCallback<bool> setter)
            {
                return Property(ValueType::kBooleanType, PropertyGetterCallbackConversion<bool>{.f2 = getter}.f4,
                                PropertySetterCallbackConversion<bool>{.f2 = setter}.f4);
            }

            template <>
            inline Property Property::CreateImpl<double>(PropertyGetterCallback<double> getter,
                                                         PropertySetterCallback<double> setter)
            {
                return Property(ValueType::kNumberType, PropertyGetterCallbackConversion<double>{.f2 = getter}.f4,
                                PropertySetterCallbackConversion<double>{.f2 = setter}.f4);
            }

            template <>
            inline Property Property::CreateImpl<std::string>(PropertyGetterCallback<std::string> getter,
                                                              PropertySetterCallback<std::string> setter)
            {
                return Property(ValueType::kStringType, PropertyGetterCallbackConversion<std::string>{.f2 = getter}.f4,
                                PropertySetterCallbackConversion<std::string>{.f2 = setter}.f4);
            }

            template <>
            inline Property Property::CreateImpl<Endpoint>(PropertyGetterCallback<Endpoint> getter,
                                                           PropertySetterCallback<Endpoint> setter)
            {
                return Property(ValueType::kEndpointType, PropertyGetterCallbackConversion<Endpoint>{.f2 = getter}.f4,
                                PropertySetterCallbackConversion<Endpoint>{.f2 = setter}.f4);
            }

            template <>
            inline Property Property::CreateImpl<Color>(PropertyGetterCallback<Color> getter,
                                                        PropertySetterCallback<Color> setter)
            {
                return Property(ValueType::kColorType, PropertyGetterCallbackConversion<Color>{.f2 = getter}.f4,
                                PropertySetterCallbackConversion<Color>{.f2 = setter}.f4);
            }

            template <class T, typename P>
            inline Property Property::Create(PropertyGetterCallback<P, T> getter, PropertySetterCallback<P, T> setter)
            {
                return Property::CreateImpl(PropertyGetterCallbackConversion<P, T>{.f1 = getter}.f2,
                                            PropertySetterCallbackConversion<P, T>{.f1 = setter}.f2);
            }
        }
    }
}