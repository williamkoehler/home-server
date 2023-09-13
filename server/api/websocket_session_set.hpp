#pragma once
#include "websocket_session.hpp"
#include "common.hpp"

namespace server
{
    namespace api
    {
        class WebSocketSession;

        class WebSocketSessionSet final
        {
          private:
            robin_hood::unordered_flat_set<WeakRef<WebSocketSession>> sessions;

          public:
            WebSocketSessionSet();
            ~WebSocketSessionSet();

            inline size_t GetSessionCount() const
            {
                return sessions.size();
            }

            bool AddSession(const Ref<WebSocketSession>& session);

            void Send(const ApiBroadcastMessage& message);
            void Send(const Ref<rapidjson::StringBuffer>& message);
            
            bool RemoveSession(const Ref<WebSocketSession>& session);
        };
    }
}