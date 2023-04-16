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

            template <class T = NativeScriptImpl>
            using MethodDefinition2 = bool (T::*)();

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

                template <class T>
                static UniqueRef<Method> Create(MethodDefinition2<T> method)
                {
                    return boost::make_unique<MethodImpl<Void, T>>(method);
                }

                virtual bool Invoke(void* self, const Value& value) = 0;
            };

#define METHOD ((static_cast<T*>(self))->*method)
#define METHOD_IMPLEMENTATION(type, condition, invokeExpr)                                                             \
    template <class T>                                                                                                 \
    class MethodImpl<type, T> final : public Method                                                                    \
    {                                                                                                                  \
      private:                                                                                                         \
        MethodDefinition<type, T> method;                                                                              \
                                                                                                                       \
      public:                                                                                                          \
        MethodImpl<type, T>(MethodDefinition<type, T> method) : method(method)                                         \
        {                                                                                                              \
        }                                                                                                              \
        virtual bool Invoke(void* self, const Value& value) override                                                   \
        {                                                                                                              \
            if (condition)                                                                                             \
                return invokeExpr;                                                                                     \
            else                                                                                                       \
            {                                                                                                          \
                LOG_WARNING("Method parameter does not match.");                                                       \
                return false;                                                                                          \
            }                                                                                                          \
        }                                                                                                              \
    };

            METHOD_IMPLEMENTATION(bool, value.IsBoolean(), METHOD(value.GetBoolean()))
            METHOD_IMPLEMENTATION(double_t, value.IsNumber(), METHOD(value.GetNumber()))
            METHOD_IMPLEMENTATION(ssize_t, value.IsInteger(), METHOD(value.GetInteger()))
            METHOD_IMPLEMENTATION(size_t, value.IsInteger(), METHOD((size_t)value.GetInteger()))
            METHOD_IMPLEMENTATION(int8_t, value.IsInteger(), METHOD((int8_t)value.GetInteger()))
            METHOD_IMPLEMENTATION(uint8_t, value.IsInteger(), METHOD((uint8_t)value.GetInteger()))
            METHOD_IMPLEMENTATION(int16_t, value.IsInteger(), METHOD((int16_t)value.GetInteger()))
            METHOD_IMPLEMENTATION(uint16_t, value.IsInteger(), METHOD((uint16_t)value.GetInteger()))
            METHOD_IMPLEMENTATION(int32_t, value.IsInteger(), METHOD((int32_t)value.GetInteger()))
            METHOD_IMPLEMENTATION(uint32_t, value.IsInteger(), METHOD((uint32_t)value.GetInteger()))
            METHOD_IMPLEMENTATION(std::string, value.IsString(), METHOD(value.GetString()))
            METHOD_IMPLEMENTATION(Endpoint, value.IsEndpoint(), METHOD(value.GetEndpoint()))
            METHOD_IMPLEMENTATION(Color, value.IsColor(), METHOD(value.GetColor()))

            template <class T>
            class MethodImpl<Value, T> final : public Method
            {
              private:
                MethodDefinition<Value, T> method;

              public:
                MethodImpl<Value, T>(MethodDefinition<Value, T> method) : method(method)
                {
                }
                virtual bool Invoke(void* self, const Value& value) override
                {
                    return METHOD(value);
                }
            };

            template <class T>
            class MethodImpl<Void, T> final : public Method
            {
              private:
                MethodDefinition2<T> method;

              public:
                MethodImpl<Void, T>(MethodDefinition2<T> method) : method(method)
                {
                }
                virtual bool Invoke(void* self, const Value& value) override
                {
                    (void)value;
                    return METHOD();
                }
            };

#undef METHOD
#undef METHOD_IMPLEMENTATION
        }
    }
}