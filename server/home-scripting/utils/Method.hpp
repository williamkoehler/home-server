#pragma once
#include "../View.hpp"
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        class Script;
        class Property;

        template <class T = Script>
        using MethodCallback = bool (T::*)(const std::string& name, Ref<Property> parameter);

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
            void Invoke(Ref<Property> parameter);

            /// @brief Post invoke to worker
            ///
            void PostInvoke(Ref<Property> parameter);
        };
    }
}