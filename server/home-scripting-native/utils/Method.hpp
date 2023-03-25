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
            using MethodFunctionCallback = bool (*)(T*, const std::string&, const P&);

            template <typename P = void*, class T = NativeScriptImpl>
            union MethodCallbackConversion
            {
                MethodCallback<P, T> f1;
                MethodFunctionCallback<P, T> f2;
                void* f3;
            };

            class Method
            {
              private:
                void* method;
                ValueType parameterType;

                template <typename P>
                static inline Method CreateImpl(void* method);

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
                inline MethodFunctionCallback<P, T> GetFunction() const
                {
                    return MethodCallbackConversion<P, T>{.f3 = method}.f2;
                }

                inline ValueType GetParameterType() const
                {
                    return parameterType;
                }
            };

            template <>
            inline Method Method::CreateImpl<Void>(void* method)
            {
                return Method(method, ValueType::kNullType);
            }

            template <>
            inline Method Method::CreateImpl<bool>(void* method)
            {
                return Method(method, ValueType::kBooleanType);
            }

            template <>
            inline Method Method::CreateImpl<double>(void* method)
            {
                return Method(method, ValueType::kBooleanType);
            }

            template <>
            inline Method Method::CreateImpl<std::string>(void* method)
            {
                return Method(method, ValueType::kStringType);
            }

            template <>
            inline Method Method::CreateImpl<Endpoint>(void* method)
            {
                return Method(method, ValueType::kEndpointType);
            }

            template <>
            inline Method Method::CreateImpl<Color>(void* method)
            {
                return Method(method, ValueType::kColorType);
            }

            template <class T, typename P>
            inline Method Method::Create(MethodCallback<P, T> method)
            {
                return Method::CreateImpl<P>(MethodCallbackConversion<P, T>{.f1 = method}.f3);
            }
        }
    }
}