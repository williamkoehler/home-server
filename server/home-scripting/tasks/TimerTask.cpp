#include "TimerTask.hpp"
#include "../Script.hpp"

namespace server
{
    namespace scripting
    {
        TimerTask::TimerTask(const Ref<Script>& script, const std::string& method, size_t interval)
            : Task(script), method(method), interval(interval), timer(Worker::GetInstance()->GetContext())
        {
        }
        TimerTask::~TimerTask()
        {
        }
        Ref<TimerTask> TimerTask::Create(const Ref<Script>& script, const std::string& method, size_t interval)
        {
            assert(script != nullptr);

            Ref<TimerTask> task = boost::make_shared<TimerTask>(script, method, interval);

            if (task != nullptr)
                task->WaitAsync();

            return task;
        }

        void TimerTask::WaitAsync()
        {
            timer.expires_from_now(boost::posix_time::seconds(std::max(interval, 1ul)));
            timer.async_wait(boost::bind(&TimerTask::TimerHandler,
                                         boost::dynamic_pointer_cast<TimerTask>(shared_from_this()),
                                         boost::placeholders::_1));
        }

        void TimerTask::TimerHandler(const boost::system::error_code& ec)
        {
            if (!ec)
            {
                if (Ref<Script> r = script.lock())
                {
                    if (r->Invoke(method, Value()))
                        WaitAsync();
                }
            }
        }

        void TimerTask::Cancel()
        {
            // Cancel timer
            boost::system::error_code ec;
            timer.cancel(ec);
        }
    }
}