#pragma once
#include "../common.hpp"
#include "Task.hpp"

namespace server
{
    namespace scripting
    {
        class TimerTask : public Task
        {
          private:
            std::string method;
            size_t interval;
            boost::asio::deadline_timer timer;

            /// @brief Start async timer
            /// 
            void WaitAsync();

            void TimerHandler(const boost::system::error_code& ec);

          public:
            TimerTask(Ref<Script> script, const std::string& method, size_t interval);
            virtual ~TimerTask();
            static Ref<TimerTask> Create(Ref<Script> script, const std::string& method, size_t interval);

            virtual void Cancel() override;
        };
    }
}