#pragma once
#include "../common.hpp"
#include "Event.hpp"

namespace server
{
    namespace scripting
    {
        class Timer : public Event
        {
          protected:
            size_t interval;
            boost::asio::deadline_timer timer;

            void TimerHandler(const boost::system::error_code& err);

          public:
            Timer(Ref<Script> script);
            virtual ~Timer();

            virtual EventType GetType() override
            {
                return EventType::kTimerEventType;
            }

            /// @brief Start timer
            /// @param interval Interval in seconds
            void Start(size_t interval);

            /// @brief Stop timer
            void Stop();
        };
    }
}