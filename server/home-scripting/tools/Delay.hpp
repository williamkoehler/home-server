#pragma once
#include "../common.hpp"
#include "Controller.hpp"

namespace server
{
    namespace scripting
    {
        class Script;

        class Delay : public Controller
        {
          private:
            CallbackMethod<> callback;

            boost::asio::deadline_timer timer;

            void Handler(boost::system::error_code ec);

          public:
            Delay(Ref<Script> script, CallbackMethod<> callback);
            virtual ~Delay();

            /// @brief Create delayed event
            ///
            /// @param script Script
            /// @param delay Delay in ms
            /// @param callback Callback
            /// @return Delay controller
            static Ref<Delay> Create(Ref<Script> script, size_t delay, CallbackMethod<> callback);

            template <class T>
            static inline Ref<Delay> Create(Ref<Script> script, size_t delay, CallbackMethod<T> callback)
            {
                return Create(script, delay, CallbackMethodConversion<T>{callback}.f2);
            }

            virtual ControllerType GetType() const override
            {
                return ControllerType::kDelayController;
            }
        };
    }
}