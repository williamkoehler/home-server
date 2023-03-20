#include "Task.hpp"
#include "../Script.hpp"

namespace server
{
    namespace scripting
    {
        Task::Task(const Ref<Script>& script) : script(script)
        {
            assert(script != nullptr);
        }
        Task::~Task()
        {
            if (Ref<Script> r = script.lock())
                r->CleanTasks();
        }
    }
}