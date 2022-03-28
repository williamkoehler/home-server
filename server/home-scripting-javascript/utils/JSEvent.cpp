#include "JSEvent.hpp"
#include "../JSScript.hpp"
#include <boost/smart_ptr/intrusive_ptr.hpp>

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            JSEvent::JSEvent(Ref<JSScript> script, const std::string& callback) : Event(script), callback(callback)
            {
                assert(script != nullptr);
            }

            void JSEvent::Invoke()
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