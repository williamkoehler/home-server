#include "Timer.hpp"
#include "../Script.hpp"

namespace server
{
    namespace scripting
    {
        Timer::Timer(Ref<Script> script, const std::string& callback)
            : script(script), callback(callback), timer(script->GetWorker()->GetContext())
        {
            assert(script != nullptr);
        }
        Timer::~Timer()
        {
        }
        Ref<Timer> Timer::Create(Ref<Script> script, const std::string& callback)
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
                    // Invoke event
                    r->Invoke(callback, shared_from_this());
                }

                if (interval != 0)
                {
                    timer.expires_from_now(boost::posix_time::seconds(interval));
                    timer.async_wait(boost::bind(&Timer::Handler,
                                                 boost::dynamic_pointer_cast<Timer>(shared_from_this()),
                                                 boost::placeholders::_1));
                }
            }
        }

        void Timer::Start(size_t i)
        {
            // Only start timer when it is not already
            if (interval == 0)
            {
                interval = i;

                timer.expires_from_now(boost::posix_time::seconds(interval));
                timer.async_wait(boost::bind(&Timer::Handler, boost::dynamic_pointer_cast<Timer>(shared_from_this()),
                                             boost::placeholders::_1));
            }
            else
            {
                interval = i;
            }
        }
        void Timer::Stop()
        {
            // Only stop timer when it is running
            if (interval != 0)
                timer.cancel();

            interval = 0;
        }
    }
}