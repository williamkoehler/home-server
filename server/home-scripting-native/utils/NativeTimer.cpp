#include "NativeTimer.hpp"
#include "../NativeScript.hpp"

namespace server
{
    namespace scripting
    {
        namespace native
        {
            NativeTimer::NativeTimer(Ref<Script> script, TimerCallback callback) : Timer(script), callback(callback)
            {
            }

            void NativeTimer::Invoke()
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