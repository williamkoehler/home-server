#pragma once
#include "../common.hpp"
#include <home-scripting/utils/Timer.hpp>

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            class JSScript;

            class JSTimer : public Timer
            {
              private:
                const std::string callback;

              public:
                JSTimer(Ref<JSScript> script, const std::string& callback);

                virtual void Invoke() override;
            };
        }
    }
}