#pragma once
#include "../View.hpp"
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        class Script;
        class Value;
        class Method;

        class Event : public boost::enable_shared_from_this<Event>
        {
          protected:
            boost::container::vector<WeakRef<Method>> methodList;

          public:
            Event();
            virtual ~Event();

            static Ref<Event> Create();

            /// @brief Add event entry
            ///
            /// @param script Script to call
            /// @param method Method to call
            void Add(Ref<Script> script, const std::string& method);

            /// @brief Invoke event
            ///
            void Invoke(Ref<Value> parameter);

            /// @brief Post invoke to worker
            ///
            void PostInvoke(Ref<Value> parameter);

            /// @brief Remove event entry
            ///
            /// @param script Script
            /// @param method Method
            void Remove(Ref<Script> script, const std::string& method);
        };
    }
}