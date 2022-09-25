#pragma once
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        class Script;

        enum class TaskType
        {
            kUnknownTask,
            kTimerTask,
            kIntervalTask,
        };

        class Task : public boost::enable_shared_from_this<Task>
        {
          protected:
            const WeakRef<Script> script;

          public:
            Task(Ref<Script> script);
            virtual ~Task();

            virtual void Cancel() = 0;
        };
    }
}