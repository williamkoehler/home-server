#include "Timer.hpp"
#include "../Script.hpp"

namespace server
{
    namespace scripting
    {
        Timer::Timer(Ref<Script> script, const std::string& event)
            : script(script), timer(script->GetWorker()->GetContext()), event(event)
        {
            assert(script != nullptr);
        }
        Timer::~Timer()
        {
        }

        void Timer::TimerHandler(const boost::system::error_code& err)
        {
            if (!err)
            {
                Ref<Script> r = script.lock();

                if (r != nullptr)
                {
                    // Invoke event
                    r->Invoke(event);
                }

                timer.expires_from_now(boost::posix_time::seconds(interval));
                timer.async_wait(boost::bind(&Timer::TimerHandler, shared_from_this(), boost::placeholders::_1));
            }
        }

        void Timer::Start(size_t i)
        {
            interval = i;

            timer.expires_from_now(boost::posix_time::seconds(interval));
            timer.async_wait(boost::bind(&Timer::TimerHandler, shared_from_this(), boost::placeholders::_1));
        }
        void Timer::Stop()
        {
            timer.cancel();
        }
    }
}