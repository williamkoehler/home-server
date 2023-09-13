#pragma once
#include "../common.hpp"
#include "../view/View.hpp"

namespace server
{
    namespace scripting
    {
        class Value;

        class Event;
        struct EventEntry;

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

            EventConnection(const Ref<EventEntry>& entry);

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
            EventConnection Bind(const Ref<View>& view, const std::string& method);

            /// @brief Invoke event
            ///
            /// @param parameter Parameter
            void Invoke(const Value& parameter) const;
        };
    }
}