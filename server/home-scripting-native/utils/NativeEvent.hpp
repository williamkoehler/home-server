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

            using EventCallback = bool (Script::*)(Ref<EventCaller> caller);

            template <class Caller>
            union EventCallbackConversion {
                bool (Caller::*method)(Ref<EventCaller>);
                EventCallback function;
            };

            class NativeEvent : public Event
            {
              private:
                EventCallback callback;

              public:
                NativeEvent(Ref<Script> script, EventCallback callback);

                /// @brief Get callback pointer
                ///
                /// @return Callback method pointer
                inline EventCallback GetCallback() const
                {
                    return callback;
                }
            };
        }
    }
}