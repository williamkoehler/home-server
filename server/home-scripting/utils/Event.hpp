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
            typedef boost::signals2::signal<void(Ref<Value>)> Signal;
            Signal signal;

          public:
            Event();
            virtual ~Event();

            static Ref<Event> Create();

            /// @brief Connect to event
            ///
            /// @param script Script to call
            /// @param method Method to call
            boost::signals2::connection Connect(Ref<Script> script, const std::string& method);

            /// @brief Invoke event
            ///
            void Invoke(Ref<Value> parameter);
        };
    }
}