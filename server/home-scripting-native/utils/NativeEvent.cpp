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

            void NativeEvent::Invoke()
            {
                Ref<Script> r = script.lock();

                if (r != nullptr)
                {
                    // Call C++ Method
                    ((r.get())->*callback)();
                }
            }
        }
    }
}