#pragma once
#include "../View.hpp"
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        class Value;

        class Event;
        class EventEntry;

        typedef boost::container::vector<Ref<EventEntry>> EventBase;

        struct EventEntry final
        {
            Ref<EventBase> base;
            WeakRef<View> view;
            std::string method;
        };

        class EventConnection final
        {
          private:
            friend class Event;

            WeakRef<EventEntry> entry;

            EventConnection(Ref<EventEntry> entry);

          public:
            EventConnection();
            EventConnection(const EventConnection& other) = delete;
            EventConnection(EventConnection&& other) noexcept;
            ~EventConnection();

            void operator=(EventConnection&& other) noexcept;

            void Unbind();
        };

        class Event final
        {
          protected:
            Ref<EventBase> base;

          public:
            Event();
            ~Event();

            /// @brief Bind script method to event
            ///
            /// @param view Invokable view
            /// @param method Method name
            /// @return EventConnection Event connection
            EventConnection Bind(Ref<View> view, const std::string& method);

            /// @brief Invoke event
            ///
            /// @param parameter Parameter
            void Invoke(const Value& parameter) const;
        };
    }
}