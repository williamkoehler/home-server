#include "NativeEvent.hpp"
#include "../NativeScript.hpp"

namespace server
{
    namespace scripting
    {
        namespace native
        {
            NativeEvent::NativeEvent(Ref<Script> script, EventCallback callback) : Event(script), callback(callback)
            {
            }
        }
    }
}