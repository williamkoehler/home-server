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

        boost::signals2::connection Event::Connect(Ref<Script> script, const std::string& method)
        {
            assert(script != nullptr);

            return signal.connect(
                Signal::slot_type(&Script::PostInvoke, script.get(), method, boost::placeholders::_1).track(script));
        }

        void Event::Invoke(Ref<Value> parameter)
        {
            signal(parameter);
        }
    }
}