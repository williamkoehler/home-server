#pragma once
#include "../common.hpp"
#include <home-scripting/utils/Event.hpp>

namespace server
{
    namespace scripting
    {
        class View;

        namespace native
        {
            class NativeScript;

            using EventCallback = bool(Script::*)();

            template <class Caller>
            struct EventCallbackConversion
            {
                EventCallback function;
                bool(Caller::*method)();
            };

            class NativeEvent : public Event
            {
              private:
                EventCallback callback;

              public:
                NativeEvent(Ref<Script> script, EventCallback callback);

                virtual void Invoke() override;
            };
        }
    }
}