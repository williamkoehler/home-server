#include "Event.hpp"
#include "../Script.hpp"

namespace server
{
    namespace scripting
    {
        Ref<EventCaller> EventCaller::Create()
        {
            static Ref<EventCaller> caller = boost::make_shared<EventCaller>();
            return caller;
        }

        Event::Event(Ref<Script> script) : script(script)
        {
            assert(script != nullptr);
        }
        Event::~Event()
        {
        }
    }
}