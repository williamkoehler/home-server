#pragma once
#include "../View.hpp"
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        class Script;
        class Value;

        template <class T = Script>
        using MethodCallback = bool (T::*)(const std::string& name, Ref<Value> parameter);

        template <class T>
        union MethodCallbackConversion
        {
            MethodCallback<T> f1;
            MethodCallback<> f2;
        };

        class Method : public boost::enable_shared_from_this<Method>
        {
          protected:
            const std::string name;
            const WeakRef<Script> script;
            const MethodCallback<> callback;

          public:
            Method(const std::string& name, Ref<Script> script, MethodCallback<> callback);
            virtual ~Method();

            static Ref<Method> Create(const std::string& name, Ref<Script> script, MethodCallback<> callback);

            template <class T>
            static inline Ref<Method> Create(const std::string& name, Ref<Script> script, MethodCallback<T> callback)
            {
                return Create(name, script, MethodCallbackConversion<T>{callback}.f2);
            }

            /// @brief Invoke method
            ///
            /// @return bool Successfulness
            bool Invoke(Ref<Value> parameter);

            /// @brief Post invoke to worker
            ///
            void PostInvoke(Ref<Value> parameter);
        };
    }
}