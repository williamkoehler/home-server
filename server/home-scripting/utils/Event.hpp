#pragma once
#include "../View.hpp"
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        class Script;

        template <class T = Script>
        using EventMethod = bool (T::*)(const std::string& event);

        template <class T>
        union EventMethodConversion
        {
            EventMethod<T> f1;
            EventMethod<> f2;
        };

        class Event : public boost::enable_shared_from_this<Event>
        {
          protected:
            std::string id;

            WeakRef<Script> script;
            EventMethod<> event;

          public:
            Event(const std::string& id, EventMethod<> event);
            virtual ~Event();

            static Ref<Event> Create(const std::string& id, EventMethod<> event);

            template <class T>
            static inline Ref<Event> Create(const std::string& id, EventMethod<T> event)
            {
                return Create(id, EventMethodConversion<T>{event}.f2);
            }

            /// @brief Get event
            ///
            /// @return Event
            inline EventMethod<> GetEvent() const
            {
                return event;
            }
        };
    }
}