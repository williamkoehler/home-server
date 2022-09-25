#include "Task.hpp"
#include "../Script.hpp"

namespace server
{
    namespace scripting
    {
        Task::Task(Ref<Script> script) : script(script)
        {
        }
        Task::~Task()
        {
            if (Ref<Script> r = script.lock())
                r->CleanTasks();
        }
    }
}