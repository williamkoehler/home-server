#pragma once
#include "../View.hpp"
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        class Script;

        struct EventEntry
        {
            const WeakRef<Script> script;
            const std::string method;
        };

        class Event : public boost::enable_shared_from_this<Event>
        {
          protected:
            const std::string& name;
            boost::container::vector<EventEntry> entryList;

          public:
            Event(const std::string& name);
            virtual ~Event();

            static Ref<Event> Create(const std::string& event);

            /// @brief Add event entry
            ///
            /// @param script Script to call
            /// @param method Method to call
            void Add(Ref<Script> script, const std::string& method);

            /// @brief Remove event entry
            ///
            /// @param script Script
            /// @param method Method
            void Remove(Ref<Script> script, const std::string& method);
        };
    }
}