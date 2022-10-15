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
            using MethodCallback = bool (T::*)(const std::string&, const P&);

            template <typename P = void*, class T = NativeScriptImpl>
            union MethodCallbackConversion
            {
                MethodCallback<P, T> f1;
                MethodCallback<P> f2;
                MethodCallback<> f3;
                void* f4;
            };

            class Method
            {
              private:
                void* method;
                ValueType parameterType;

                template <typename P>
                static inline Method CreateImpl(MethodCallback<P> methodr);

              public:
                Method();
                Method(void* method, ValueType parameterType);
                ~Method();
                template <class T, typename P>
                static inline Method Create(MethodCallback<P, T> method);

                inline void* GetMethod() const
                {
                    return method;
                }

                template <typename P, class T = NativeScriptImpl>
                inline MethodCallback<P, T> GetMethod() const
                {
                    return MethodCallbackConversion<P, T>{.f4 = method}.f1;
                }

                inline ValueType GetParameterType() const
                {
                    return parameterType;
                }
            };

            template <>
            inline Method Method::CreateImpl<Void>(MethodCallback<Void> method)
            {
                return Method(MethodCallbackConversion<Void>{.f2 = method}.f4, ValueType::kNullType);
            }

            template <>
            inline Method Method::CreateImpl<bool>(MethodCallback<bool> method)
            {
                return Method(MethodCallbackConversion<bool>{.f2 = method}.f4, ValueType::kBooleanType);
            }

            template <>
            inline Method Method::CreateImpl<double>(MethodCallback<double> method)
            {
                return Method(MethodCallbackConversion<double>{.f2 = method}.f4, ValueType::kBooleanType);
            }

            template <>
            inline Method Method::CreateImpl<std::string>(MethodCallback<std::string> method)
            {
                return Method(MethodCallbackConversion<std::string>{.f2 = method}.f4, ValueType::kStringType);
            }

            template <>
            inline Method Method::CreateImpl<Endpoint>(MethodCallback<Endpoint> method)
            {
                return Method(MethodCallbackConversion<Endpoint>{.f2 = method}.f4, ValueType::kEndpointType);
            }

            template <>
            inline Method Method::CreateImpl<Color>(MethodCallback<Color> method)
            {
                return Method(MethodCallbackConversion<Color>{.f2 = method}.f4, ValueType::kColorType);
            }

            template <class T, typename P>
            inline Method Method::Create(MethodCallback<P, T> method)
            {
                return Method::CreateImpl(MethodCallbackConversion<P, T>{.f1 = method}.f2);
            }
        }
    }
}