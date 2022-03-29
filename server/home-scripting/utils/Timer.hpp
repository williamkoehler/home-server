#pragma once
#include "../common.hpp"
#include "Event.hpp"

namespace server
{
    namespace scripting
    {
        class Timer : public boost::enable_shared_from_this<Timer>
        {
          protected:
            WeakRef<Script> script;

            size_t interval;
            boost::asio::deadline_timer timer;

            std::string event;

            void TimerHandler(const boost::system::error_code& err);

          public:
            Timer(Ref<Script> script, const std::string& event);
            virtual ~Timer();

            /// @brief Start timer
            /// @param interval Interval in seconds
            void Start(size_t interval);

            /// @brief Stop timer
            void Stop();
        };
    }
}