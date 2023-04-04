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
            using CallbackMethodDefinition = bool (T::*)(const P&);

            template <typename P, class T>
            class CallbackMethodImpl;

            template <typename P>
            class CallbackMethod
            {
              public:
                CallbackMethod()
                {
                }
                virtual ~CallbackMethod()
                {
                }

                template <class T>
                static inline UniqueRef<CallbackMethod<P>> Create(CallbackMethodDefinition<P, T> method)
                {
                    assert(method != nullptr);
                    return boost::make_unique<CallbackMethodImpl<P, T>>(method);
                }

                virtual bool Invoke(void* self, P value) = 0;
            };

            template <typename P, class T>
            class CallbackMethodImpl final : public CallbackMethod<P>
            {
              private:
                CallbackMethodDefinition<P, T> method;

              public:
                CallbackMethodImpl(CallbackMethodDefinition<P, T> method) : method(method)
                {
                }
                virtual bool Invoke(void* self, P value) override
                {
                    return (static_cast<T*>(self)->*method)(value);
                }
            };
        }
    }
}