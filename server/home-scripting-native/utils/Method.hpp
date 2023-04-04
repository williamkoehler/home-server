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
            using MethodDefinition = bool (T::*)(const P&);

            template <typename P, class T>
            class MethodImpl;

            class Method
            {
              public:
                Method()
                {
                }
                virtual ~Method()
                {
                }

                template <typename P, class T>
                static UniqueRef<Method> Create(MethodDefinition<P, T> method)
                {
                    return boost::make_unique<MethodImpl<P, T>>(method);
                }

                virtual bool Invoke(void* self, const Value& value) = 0;
            };

#define METHOD ((static_cast<T*>(self))->*method)
#define METHOD_IMPLEMENTATION(type, condition, invokeExpr)                                                             \
    template <class T>                                                                                                 \
    class MethodImpl<type, T> final : public Method                                                                    \
    {                                                                                                                  \
      private:                                                                                                         \
        MethodDefinition<type, T> method;                                                                                \
                                                                                                                       \
      public:                                                                                                          \
        MethodImpl<type, T>(MethodDefinition<type, T> method) : method(method)                                           \
        {                                                                                                              \
        }                                                                                                              \
        virtual bool Invoke(void* self, const Value& value) override                                                   \
        {                                                                                                              \
            if (condition)                                                                                             \
                return invokeExpr;                                                                                     \
            else                                                                                                       \
                return false;                                                                                          \
        }                                                                                                              \
    };

            METHOD_IMPLEMENTATION(bool, value.IsBoolean(), METHOD(value.GetBoolean()))
            METHOD_IMPLEMENTATION(double_t, value.IsNumber(), METHOD(value.GetNumber()))
            METHOD_IMPLEMENTATION(size_t, value.IsNumber(), METHOD((size_t)value.GetNumber()))
            METHOD_IMPLEMENTATION(int8_t, value.IsNumber(), METHOD((int8_t)value.GetNumber()))
            METHOD_IMPLEMENTATION(uint8_t, value.IsNumber(), METHOD((uint8_t)value.GetNumber()))
            METHOD_IMPLEMENTATION(int16_t, value.IsNumber(), METHOD((int16_t)value.GetNumber()))
            METHOD_IMPLEMENTATION(uint16_t, value.IsNumber(), METHOD((uint16_t)value.GetNumber()))
            METHOD_IMPLEMENTATION(int32_t, value.IsNumber(), METHOD((int32_t)value.GetNumber()))
            METHOD_IMPLEMENTATION(uint32_t, value.IsNumber(), METHOD((uint32_t)value.GetNumber()))
            METHOD_IMPLEMENTATION(std::string, value.IsString(), METHOD(value.GetString()))
            METHOD_IMPLEMENTATION(Endpoint, value.IsEndpoint(), METHOD(value.GetEndpoint()))
            METHOD_IMPLEMENTATION(Color, value.IsColor(), METHOD(value.GetColor()))

#undef METHOD
#undef METHOD_IMPLEMENTATION
        }
    }
}