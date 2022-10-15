#pragma once
#include "../View.hpp"
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        class Script;
        class Value;
        class Method;

        class Event;

        class EventConnection
        {
          private:
            friend class Event;

            boost::signals2::connection connection;

            EventConnection(boost::signals2::connection connection) : connection(connection)
            {
            }
          public:
            EventConnection(EventConnection&& eventConnection)
            {
                connection.swap(eventConnection.connection);
            }
            ~EventConnection()
            {
                connection.disconnect();
            }

            inline void Disconnect()
            {
                connection.disconnect();
            }
        };

        class Event : public boost::enable_shared_from_this<Event>
        {
          protected:
            typedef boost::signals2::signal<void(const Value&)> Signal;
            Signal signal;

          public:
            Event();
            virtual ~Event();

            static Ref<Event> Create();

            /// @brief Connect to event
            ///
            /// @param script Script to call
            /// @param method Method to call
            EventConnection Connect(Ref<Script> script, const std::string& method);

            /// @brief Invoke event
            ///
            void Invoke(const Value& parameter);
        };
    }
}