#pragma once
#include "../common.hpp"
#include <home-scripting/utils/Event.hpp>

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            class JSScript;

            class JSEvent : public Event
            {
              private:
                const std::string callback;

              public:
                JSEvent(Ref<JSScript> script, const std::string& callback);

                virtual void Invoke() override;
            };
        }
    }
}