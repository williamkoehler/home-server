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

        void Event::PostInvoke()
        {
            Ref<Script> r = script.lock();

            if (r != nullptr)
                r->GetWorker()->GetContext().post(boost::bind(&Event::Invoke, shared_from_this()));
        }
    }
}