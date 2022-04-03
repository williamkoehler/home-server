#pragma once
#include "../common.hpp"
#include "Controller.hpp"

namespace server
{
    namespace scripting
    {
        class Script;

        class Timer : public Controller
        {
          private:
            CallbackMethod<> callback;
            size_t interval;

            boost::asio::deadline_timer timer;

            void Handler(boost::system::error_code ec);

          public:
            Timer(Ref<Script> script, CallbackMethod<> callback);
            virtual ~Timer();
            static Ref<Timer> Create(Ref<Script> script, CallbackMethod<> callback);

            template <class T>
            static inline Ref<Timer> Create(Ref<Script> script, CallbackMethod<T> callback)
            {
                return Create(script, CallbackMethodConversion<T>{callback}.f2);
            }

            virtual ControllerType GetType() const override
            {
                return ControllerType::kTimerController;
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