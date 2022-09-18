#include "Event.hpp"
#include "../Script.hpp"

namespace server
{
    namespace scripting
    {
        Event::Event(const std::string& name) : name(name)
        {
        }
        Event::~Event()
        {
        }

        Ref<Event> Event::Create(const std::string& name)
        {
            return boost::make_shared<Event>(name);
        }

        void Event::Add(Ref<Script> script, const std::string& method)
        {
            entryList.push_back(EventEntry{
                script,
                method,
            });
        }

        void Event::Remove(Ref<Script> script, const std::string& method)
        {
            for (boost::container::vector<EventEntry>::iterator it = entryList.begin(); it != entryList.end(); it++)
            {
                // if (it->script.lock() == script && it->method == method)
                //     it = entryList.erase(it);
            }
        }
    }
}