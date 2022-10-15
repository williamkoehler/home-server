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

        EventConnection Event::Connect(Ref<Script> script, const std::string& method)
        {
            assert(script != nullptr);

            return EventConnection(signal.connect(
                Signal::slot_type(&Script::PostInvoke, script.get(), method, boost::placeholders::_1).track(script)));
        }

        void Event::Invoke(const Value& parameter)
        {
            signal(parameter);
        }
    }
}