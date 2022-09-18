#include "Timer.hpp"
#include "../Script.hpp"

namespace server
{
    namespace scripting
    {
        Timer::Timer(Ref<Script> script, CallbackMethod<> callback)
            : Controller(script), callback(callback), timer(script->GetWorker()->GetContext())
        {
            assert(script != nullptr);
        }
        Timer::~Timer()
        {
        }
        Ref<Timer> Timer::Create(Ref<Script> script, CallbackMethod<> callback)
        {
            return boost::make_shared<Timer>(script, callback);
        }

        void Timer::Handler(boost::system::error_code err)
        {
            if (!err)
            {
                Ref<Script> r = script.lock();

                if (r != nullptr)
                {
                    // Invoke callback
                    (r.get()->*callback)(shared_from_this());
                }
            }
        }

        void Timer::Start(size_t interval)
        {
            timer.expires_from_now(boost::posix_time::seconds(interval));
            timer.async_wait(boost::bind(&Timer::Handler, boost::dynamic_pointer_cast<Timer>(shared_from_this()),
                                         boost::placeholders::_1));
        }

        void Timer::Stop()
        {
            timer.cancel();
        }
    }
}