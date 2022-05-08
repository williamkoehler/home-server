#include "Delay.hpp"
#include "../Script.hpp"

namespace server
{
    namespace scripting
    {
        Delay::Delay(Ref<Script> script, CallbackMethod<> callback)
            : Controller(script), callback(callback), timer(script->GetWorker()->GetContext())
        {
            assert(script != nullptr);
        }
        Delay::~Delay()
        {
        }
        Ref<Delay> Delay::Create(Ref<Script> script, size_t delayMS, CallbackMethod<> callback)
        {
            Ref<Delay> delay = boost::make_shared<Delay>(script, callback);

            if (delay != nullptr)
            {
                delay->timer.expires_from_now(boost::posix_time::milliseconds(delayMS));
                delay->timer.async_wait(
                    boost::bind(&Delay::Handler, boost::dynamic_pointer_cast<Delay>(delay), boost::placeholders::_1));
            }

            return delay;
        }

        void Delay::Handler(boost::system::error_code err)
        {
            if (!err)
            {
                try
                {
                    Ref<Script> r = script.lock();

                    if (r != nullptr)
                        ((r.get())->*callback)(shared_from_this());
                }
                catch (std::exception)
                {
                }
            }
        }
    }
}