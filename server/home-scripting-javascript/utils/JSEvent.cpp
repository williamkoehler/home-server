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
        }
    }
}