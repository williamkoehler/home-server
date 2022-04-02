#pragma once
#include "../common.hpp"
#include "../utils/Event.hpp"

namespace server
{
    namespace scripting
    {
        class Script;

        class Timer : public EventCaller
        {
          private:
            WeakRef<Script> script;
            std::string callback;
            size_t interval;

            boost::asio::deadline_timer timer;

            void Handler(boost::system::error_code ec);

          public:
            Timer(Ref<Script> script, const std::string& callback);
            virtual ~Timer();
            static Ref<Timer> Create(Ref<Script> script, const std::string& callback);

            virtual EventCallerType GetType() const override
            {
                return EventCallerType::kTimerEventCaller;
            }

            /// @brief Is timer running
            ///
            /// @return Timer state
            inline bool IsRunning() const
            {
                return interval != 0;
            }

            /// @brief Get timer interval
            ///
            /// @return Interval in seconds
            inline size_t GetInterval() const
            {
                return interval;
            }

            /// @brief Start timer
            ///
            /// @param interval Interval in seconds
            void Start(size_t interval);

            /// @brief Stop timer
            ///
            void Stop();
        };
    }
}