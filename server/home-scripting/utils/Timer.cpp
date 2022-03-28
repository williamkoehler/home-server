#include "Timer.hpp"
#include "../Script.hpp"
#include <boost/smart_ptr/intrusive_ptr.hpp>

namespace server
{
    namespace scripting
    {
        Timer::Timer(Ref<Script> script) : Event(script), timer(script->GetWorker()->GetContext())
        {
        }
        Timer::~Timer()
        {
        }

        void Timer::TimerHandler(const boost::system::error_code& err)
        {
            if (!err)
            {
                // Invoke timer
                Invoke();

                timer.expires_from_now(boost::posix_time::seconds(interval));
                timer.async_wait(boost::bind(&Timer::TimerHandler,
                                             boost::dynamic_pointer_cast<Timer>(shared_from_this()),
                                             boost::placeholders::_1));
            }
        }

        void Timer::Start(size_t i)
        {
            interval = i;

            timer.expires_from_now(boost::posix_time::seconds(interval));
            timer.async_wait(boost::bind(&Timer::TimerHandler, boost::dynamic_pointer_cast<Timer>(shared_from_this()),
                                         boost::placeholders::_1));
        }
        void Timer::Stop()
        {
            timer.cancel();
        }
    }
}