#include "Event.hpp"
#include "../Script.hpp"

namespace server
{
    namespace scripting
    {
        Event::Event(const std::string& id, EventMethod<> event) : id(id), event(event)
        {
            assert(event != nullptr);
        }
        Event::~Event()
        {
        }

        Ref<Event> Event::Create(const std::string& id, EventMethod<> event)
        {
            return boost::make_shared<Event>(id, event);
        }
    }
}