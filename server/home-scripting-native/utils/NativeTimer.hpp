#pragma once
#include "../common.hpp"
#include <home-scripting/utils/Timer.hpp>

namespace server
{
    namespace scripting
    {
        class View;

        namespace native
        {
            class NativeScript;

            using TimerCallback = bool (Script::*)();

            template <class Caller>
            struct TimerCallbackConversion
            {
                TimerCallback function;
                bool (Caller::*method)();
            };

            class NativeTimer : public Timer
            {
              private:
                TimerCallback callback;

              public:
                NativeTimer(Ref<Script> script, TimerCallback callback);

                virtual void Invoke() override;
            };
        }
    }
}