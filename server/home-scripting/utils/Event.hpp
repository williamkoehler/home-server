#pragma once
#include "../common.hpp"
#include "../View.hpp"

namespace server
{
    namespace scripting
    {
        class Script;

        class Event : public boost::enable_shared_from_this<Event>
        {
          protected:
            WeakRef<Script> script;

          public:
            Event(Ref<Script> script);
            virtual ~Event();
        };
    }
}