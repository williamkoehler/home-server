#include "websocket_session_set.hpp"

namespace server
{
    namespace api
    {
        WebSocketSessionSet::WebSocketSessionSet() : sessions()
        {
        }
        WebSocketSessionSet::~WebSocketSessionSet()
        {
        }

        bool WebSocketSessionSet::AddSession(const Ref<WebSocketSession>& session)
        {
            return sessions.insert(session).second;
        }

        void WebSocketSessionSet::Send(const ApiBroadcastMessage& message)
        {
            Ref<rapidjson::StringBuffer> buffer = boost::make_shared<rapidjson::StringBuffer>();
            if (buffer != nullptr)
            {
                // Build message
                {
                    rapidjson::Writer<rapidjson::StringBuffer> writer =
                        rapidjson::Writer<rapidjson::StringBuffer>(*buffer);

                    writer.StartObject();

                    // Message id field
                    writer.Key("msgid", 5);
                    writer.Uint64(0);

                    // Message type field
                    {
                        writer.Key("msg", 3);
                        const std::string& type = message.GetType();
                        writer.String(type.data(), type.size(), true);
                    }

                    // Content field
                    {
                        const rapidjson::Document& document = message.GetJsonDocument();
                        for (rapidjson::Value::ConstMemberIterator memberIt = document.MemberBegin();
                             memberIt != document.MemberEnd(); memberIt++)
                        {
                            writer.Key(memberIt->name.GetString(), memberIt->name.GetStringLength());
                            memberIt->value.Accept(writer);
                        }
                    }
                    writer.EndObject(3);
                }

                Send(buffer);
            }
            else
            {
                LOG_ERROR("Failed to create string buffer.");
            }
        }

        void WebSocketSessionSet::Send(const Ref<rapidjson::StringBuffer>& message)
        {
            assert(message != nullptr);

            robin_hood::unordered_flat_set<WeakRef<WebSocketSession>>::const_iterator it = sessions.begin();
            while (it != sessions.end())
            {
                if (Ref<WebSocketSession> session = (*it).lock())
                {
                    session->Send(message);

                    it++; // Next session
                }
                else
                {
                    // Remove session and move iterator
                    it = sessions.erase(it);
                }
            }
        }

        bool WebSocketSessionSet::RemoveSession(const Ref<WebSocketSession>& session)
        {
            return sessions.erase(session);
        }
    }
}