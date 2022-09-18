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