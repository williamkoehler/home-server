#pragma once
#include "../common.hpp"
#include "../View.hpp"

namespace server
{
    namespace scripting
    {
        class Script;

        enum class EventType
        {
            kUnknownEventType,
            kSimpleEventType,
            kTimerEventType,
        };

        class Event : public boost::enable_shared_from_this<Event>
        {
          protected:
            WeakRef<Script> script;

          public:
            Event(Ref<Script> script);
            virtual ~Event();

            virtual EventType GetType()
            {
                return EventType::kSimpleEventType;
            }

            /// @brief Invoke event callback
            /// 
            virtual void Invoke() = 0;

            /// @brief Invoke event callback from another thread
            ///
            void PostInvoke();
        };
    }
}