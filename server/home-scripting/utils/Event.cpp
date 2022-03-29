#include "Event.hpp"
#include "../Script.hpp"

namespace server
{
    namespace scripting
    {
        Event::Event(Ref<Script> script) : script(script)
        {
            assert(script != nullptr);
        }
        Event::~Event()
        {
        }
    }
}