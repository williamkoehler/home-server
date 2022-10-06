#include "Event.hpp"
#include "../Script.hpp"

namespace server
{
    namespace scripting
    {
        Event::Event()
        {
        }
        Event::~Event()
        {
        }

        Ref<Event> Event::Create()
        {
            return boost::make_shared<Event>();
        }

        void Event::Add(Ref<Script> script, const std::string& method)
        {
            assert(script != nullptr);

            if (script != nullptr)
                entryList.push_back(EventEntry{.script = script, .method = method});
        }

        void Event::Invoke(Ref<Value> parameter)
        {
            for (boost::container::vector<EventEntry>::iterator it = entryList.begin(); it != entryList.end(); it++)
            {
                Ref<Script> script = it->script.lock();

                if (script != nullptr)
                    script->Invoke(it->method, parameter);
                else
                    it = entryList.erase(it);
            }
        }

        void Event::PostInvoke(Ref<Value> parameter)
        {
            for (boost::container::vector<EventEntry>::iterator it = entryList.begin(); it != entryList.end(); it++)
            {
                Ref<Script> script = it->script.lock();

                if (script != nullptr)
                    script->PostInvoke(it->method, parameter);
                else
                    it = entryList.erase(it);
            }
        }

        void Event::Remove(Ref<Script> script, const std::string& method)
        {
            assert(script != nullptr);

            for (boost::container::vector<EventEntry>::iterator it = entryList.begin(); it != entryList.end(); it++)
            {
                Ref<Script> script2 = it->script.lock();

                if (script2 == script || script2 == nullptr)
                    it = entryList.erase(it);
            }
        }
    }
}