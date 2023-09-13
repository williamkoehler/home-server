#include "websocket_session.hpp"
#include "network_manager.hpp"

namespace server
{
    namespace api
    {
        robin_hood::unordered_node_map<std::string, WebSocketApiCallDefinition> webSocketApiMap =
            robin_hood::unordered_node_map<std::string, WebSocketApiCallDefinition>();
        // };

        robin_hood::unordered_node_map<std::string, WebSocketApiCallDefinition>& WebSocketSession::GetApiMap()
        {
            return webSocketApiMap;
        }

        WebSocketSession::WebSocketSession(const Ref<tcp_socket_t>& socket, const Ref<api::User>& user)
            : strand(socket->get_executor()), user(user), socket(boost::make_shared<websocket_t>(std::move(*socket)))
        {
        }
        WebSocketSession::~WebSocketSession()
        {
        }

        void WebSocketSession::Run(boost::beast::http::request<boost::beast::http::string_body>& request)
        {
            socket->next_layer().expires_after(std::chrono::seconds(12));
            socket->set_option(boost::beast::websocket::stream_base::decorator(
                [](boost::beast::websocket::response_type& response) -> void
                { response.set(boost::beast::http::field::server, "HomeAutomation Server WebSocket"); }));
            socket->async_accept(
                request, boost::asio::bind_executor(strand, boost::bind(&WebSocketSession::OnAccept, shared_from_this(),
                                                                        boost::placeholders::_1)));
        }

        void WebSocketSession::OnAccept(const boost::system::error_code& ec)
        {
            if (ec)
                return;

            socket->next_layer().expires_never();

            socket->text(true);
            socket->async_read(buffer, boost::asio::bind_executor(
                                           strand, boost::bind(&WebSocketSession::OnRead, shared_from_this(),
                                                               boost::placeholders::_1, boost::placeholders::_2)));
        }

        void WebSocketSession::OnRead(const boost::system::error_code& ec, size_t receivedBytes)
        {
            if (ec)
                return;

            if (!socket->got_text())
            {
                DoWSShutdown(boost::beast::websocket::close_code::bad_payload);
                return;
            }
            if (!socket->is_message_done())
            {
                socket->async_read(buffer, boost::asio::bind_executor(
                                               strand, boost::bind(&WebSocketSession::OnRead, shared_from_this(),
                                                                   boost::placeholders::_1, boost::placeholders::_2)));
                return;
            }

            buffer.reserve(buffer.size() + 1);
            char* data = static_cast<char*>(buffer.data().data());
            data[buffer.size()] = '\0';
            rapidjson::StringStream stream = rapidjson::StringStream(static_cast<const char*>(buffer.data().data()));

            // Parse request
            size_t id;
            ApiRequestMessage request = ApiRequestMessage();
            {
                rapidjson::Document& requestDocument = request.GetJsonDocument();

                requestDocument.ParseStream(stream);
                if (requestDocument.HasParseError() || !requestDocument.IsObject())
                {
                    DoWSShutdown(boost::beast::websocket::close_code::bad_payload, "Invalid JSON");
                    return;
                }

                buffer.consume(receivedBytes);

                rapidjson::Value::MemberIterator messageIdIt = requestDocument.FindMember("msgid");
                rapidjson::Value::MemberIterator messageIt = requestDocument.FindMember("msg");
                if (messageIdIt == requestDocument.MemberEnd() || !messageIdIt->value.IsUint64() ||
                    messageIt == requestDocument.MemberEnd() || !messageIt->value.IsString())
                {
                    DoWSShutdown(boost::beast::websocket::close_code::bad_payload, "Invalid JSON");
                    return;
                }

                id = messageIdIt->value.GetUint64();
                request.GetType().assign(messageIt->value.GetString(), messageIt->value.GetStringLength());
            }

            ApiResponseMessage response = ApiResponseMessage();

            // Call websocket message
            robin_hood::unordered_node_map<std::string, WebSocketApiCallDefinition>::const_iterator it =
                webSocketApiMap.find(request.GetType());
            if (it != webSocketApiMap.end())
                it->second(user, request, response, shared_from_this());
            else
                response.SetErrorCode(kApiErrorCode_InvalidArguments);

            Send(id, response);

            // Wait for data
            socket->async_read(buffer, boost::asio::bind_executor(
                                           strand, boost::bind(&WebSocketSession::OnRead, shared_from_this(),
                                                               boost::placeholders::_1, boost::placeholders::_2)));
        }

        void WebSocketSession::Send(size_t id, const ApiResponseMessage& message)
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
                    writer.Uint64(id);

                    // Message type field
                    writer.Key("msg", 3);

                    if (message.GetErrorCode() == kApiErrorCode_NoError)
                        writer.String("ack", 3);
                    else
                        writer.String("nack", 4);

                    // Message error code field
                    writer.Key("error", 5);
                    writer.Uint64((uint64_t)message.GetErrorCode());

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

                messageQueue.push_back(buffer);

                if (messageQueue.size() == 1)
                {
                    socket->async_write(
                        boost::asio::buffer(buffer->GetString(), buffer->GetSize()),
                        boost::asio::bind_executor(strand, boost::bind(&WebSocketSession::OnWrite, shared_from_this(),
                                                                       boost::placeholders::_1, boost::placeholders::_2,
                                                                       buffer)));
                }
            }
            else
            {
                LOG_ERROR("Failed to create string buffer.");
            }
        }

        void WebSocketSession::Send(const Ref<rapidjson::StringBuffer>& buffer)
        {
            if (buffer != nullptr)
            {
                messageQueue.push_back(buffer);

                if (messageQueue.size() == 1)
                {
                    socket->async_write(
                        boost::asio::buffer(buffer->GetString(), buffer->GetSize()),
                        boost::asio::bind_executor(strand, boost::bind(&WebSocketSession::OnWrite, shared_from_this(),
                                                                       boost::placeholders::_1, boost::placeholders::_2,
                                                                       buffer)));
                }
            }
        }

        void WebSocketSession::OnWrite(const boost::system::error_code& ec, size_t sentBytes,
                                       const Ref<rapidjson::StringBuffer>& message)
        {
            (void)sentBytes;
            (void)message;

            if (ec)
                return;

            messageQueue.erase(messageQueue.begin());

            if (messageQueue.size())
            {
                Ref<rapidjson::StringBuffer> message = messageQueue.front();

                socket->async_write(
                    boost::asio::buffer(message->GetString(), message->GetSize()),
                    boost::asio::bind_executor(strand,
                                               boost::bind(&WebSocketSession::OnWrite, shared_from_this(),
                                                           boost::placeholders::_1, boost::placeholders::_2, message)));
            }
        }

        void WebSocketSession::DoWSShutdown(boost::beast::websocket::close_code code, const char* reason)
        {
            // Shutdown
            socket->next_layer().expires_after(std::chrono::seconds(6));
            socket->async_close(
                boost::beast::websocket::close_reason(code, reason),
                boost::asio::bind_executor(
                    strand, boost::bind(&WebSocketSession::OnShutdown, shared_from_this(), boost::placeholders::_1)));
        }
        void WebSocketSession::DoSSLShutdown(const boost::system::error_code& ec)
        {
            if (ec)
                return;

            // // Shutdown
            // socket->next_layer().expires_after(std::chrono::seconds(6));
            // socket->next_layer().async_shutdown(boost::asio::bind_executor(
            //     strand, boost::bind(&WSSession::OnShutdown, shared_from_this(), boost::placeholders::_1)));
        }
        void WebSocketSession::OnShutdown(const boost::system::error_code& ec)
        {
            if (ec)
                return;

            socket->next_layer().close();
        }
    }
}