#pragma once
#include "../View.hpp"
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        class Script;

        enum class EventCallerType
        {
            kEmptyEventCaller,
            kTimerEventCaller,
            kHttpResponseEventCaller,
        };

        class EventCaller : public boost::enable_shared_from_this<EventCaller>
        {
          public:
            /// @brief Create empty event args. 
            /// Note: Only returns a static reference.
            ///
            /// @return Empty event args
            static Ref<EventCaller> Create();

            /// @brief Get event caller type
            /// 
            /// @return Event caller type
            virtual EventCallerType GetType() const
            {
                return EventCallerType::kEmptyEventCaller;
            };
        };

        class Event : public boost::enable_shared_from_this<Event>
        {
          protected:
            WeakRef<Script> script;

          public:
            Event(Ref<Script> script);
            virtual ~Event();
        };
    }
}