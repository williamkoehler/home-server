#include "JSTimer.hpp"
#include "../JSScript.hpp"

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            JSTimer::JSTimer(Ref<JSScript> script, const std::string& callback) : Timer(script), callback(callback)
            {
            }

            void JSTimer::Invoke()
            {
                Ref<JSScript> r = boost::dynamic_pointer_cast<JSScript>(script.lock());

                if (r != nullptr)
                {
                    // Post job
                    r->GetWorker()->GetContext().post(boost::bind(&JSScript::InvokeImpl, r, callback));
                }
            }
        }
    }
}