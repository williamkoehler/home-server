#pragma once
#include "../View.hpp"
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        class Script;

        template <class T = Script>
        using MethodCallback = bool (T::*)(const std::string& method);

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
            Method(const std::string& name, MethodCallback<> callback);
            virtual ~Method();

            static Ref<Method> Create(const std::string& name, MethodCallback<> callback);

            template <class T>
            static inline Ref<Method> Create(const std::string& name, MethodCallback<T> callback)
            {
                return Create(name, MethodCallbackConversion<T>{callback}.f2);
            }

            /// @brief Get callback
            ///
            /// @return Callback
            inline MethodCallback<> GetCallback() const
            {
                return callback;
            }
        };
    }
}