#include "WSSession.hpp"
#include "NetworkManager.hpp"
#include "json/JsonApi.hpp"

namespace server
{
    namespace networking
    {
        WSSession::WSSession(Ref<ssl_socket_t> socket, Ref<users::User> user)
            : strand(socket->get_executor()), socket(boost::make_shared<websocket_t>(std::move(*socket))), user(user)
        {
        }
        WSSession::~WSSession()
        {
        }

        void WSSession::Run(boost::beast::http::request<boost::beast::http::string_body>& request)
        {
            socket->next_layer().next_layer().expires_after(std::chrono::seconds(12));
            socket->set_option(boost::beast::websocket::stream_base::decorator(
                [](boost::beast::websocket::response_type& response) -> void {
                    response.set(boost::beast::http::field::server, "HomeAutomation Server WebSocket");
                }));
            socket->async_accept(
                request, boost::asio::bind_executor(
                             strand, boost::bind(&WSSession::OnAccept, shared_from_this(), boost::placeholders::_1)));
        }

        void WSSession::OnAccept(boost::system::error_code error)
        {
            if (error)
                return;

            socket->next_layer().next_layer().expires_never();

            // Add ws to publish list
            {
                Ref<NetworkManager> networkManager = NetworkManager::GetInstance();
                boost::lock_guard lock(networkManager->mutex);
                networkManager->sessionList.push_back(shared_from_this());
            }

            socket->text(true);
            socket->async_read(buffer, boost::asio::bind_executor(
                                           strand, boost::bind(&WSSession::OnRead, shared_from_this(),
                                                               boost::placeholders::_1, boost::placeholders::_2)));
        }

        void WSSession::OnRead(boost::system::error_code error, size_t receivedBytes)
        {
            if (error)
                return;

            if (!socket->got_text())
            {
                DoWSShutdown(boost::beast::websocket::close_code::bad_payload);
                return;
            }
            if (!socket->is_message_done())
            {
                socket->async_read(buffer, boost::asio::bind_executor(
                                               strand, boost::bind(&WSSession::OnRead, shared_from_this(),
                                                                   boost::placeholders::_1, boost::placeholders::_2)));
                return;
            }

            buffer.reserve(buffer.size() + 1);
            char* data = static_cast<char*>(buffer.data().data());
            data[buffer.size()] = '\0';
            rapidjson::StringStream stream = rapidjson::StringStream(static_cast<const char*>(buffer.data().data()));

            rapidjson::Document requestDocument;
            requestDocument.ParseStream(stream);

            if (requestDocument.HasParseError() || !requestDocument.IsObject())
            {
                DoWSShutdown(boost::beast::websocket::close_code::bad_payload, "Invalid JSON");
                return;
            }

            buffer.consume(receivedBytes);

            rapidjson::Value::MemberIterator messageIDIt = requestDocument.FindMember("msgid");
            rapidjson::Value::MemberIterator messageIt = requestDocument.FindMember("msg");
            if (messageIDIt == requestDocument.MemberEnd() || !messageIDIt->value.IsUint() ||
                messageIt == requestDocument.MemberEnd() || !messageIt->value.IsString())
            {
                DoWSShutdown(boost::beast::websocket::close_code::bad_payload, "Invalid JSON");
                return;
            }

            rapidjson::Document responseDocument = rapidjson::Document(rapidjson::kObjectType);

            if (ProcessJsonApi(messageIDIt->value.GetUint(),
                               std::string(messageIt->value.GetString(), messageIt->value.GetStringLength()),
                               requestDocument, responseDocument))
                Send(responseDocument);

            // Wait for data
            socket->async_read(buffer, boost::asio::bind_executor(
                                           strand, boost::bind(&WSSession::OnRead, shared_from_this(),
                                                               boost::placeholders::_1, boost::placeholders::_2)));
        }

        typedef void(WSMethod)(const Ref<users::User>&, rapidjson::Document&, rapidjson::Document&, ApiContext&);
        static const boost::unordered::unordered_map<std::string, WSMethod*> methodList = {
            // User
            {"get-users", JsonApi::ProcessJsonGetUsersMessageWS},
            {"add-user", JsonApi::ProcessJsonAddUserMessageWS},
            {"rem-user", JsonApi::ProcessJsonRemoveUserMessageWS},

            // Scripting
            {"get-scriptsources", JsonApi::ProcessJsonGetScriptSourcesMessageWS},

            {"add-scriptsource", JsonApi::ProcessJsonAddScriptSourceMessageWS},

            {"get-scriptsource", JsonApi::ProcessJsonGetScriptSourceMessageWS},
            {"get-scriptsource?content", JsonApi::ProcessJsonGetScriptSourceContentMessageWS},

            {"set-scriptsource", JsonApi::ProcessJsonSetScriptSourceMessageWS},
            {"set-scriptsource?content", JsonApi::ProcessJsonSetScriptSourceContentMessageWS},

            {"rem-scriptsource", JsonApi::ProcessJsonRemoveScriptSourceMessageWS},

            // Home
            {"get-home", JsonApi::ProcessJsonGetHomeMessageWS},

            // Room
            {"add-room", JsonApi::ProcessJsonAddRoomMessageWS},

            {"get-room", JsonApi::ProcessJsonGetRoomMessageWS},
            {"set-room", JsonApi::ProcessJsonSetRoomMessageWS},

            {"rem-room", JsonApi::ProcessJsonRemoveRoomMessageWS},

            // Device
            {"add-device", JsonApi::ProcessJsonAddDeviceMessageWS},

            {"inv-device", JsonApi::ProcessJsonInvokeDeviceEventMessageWS},

            {"get-device", JsonApi::ProcessJsonGetDeviceMessageWS},
            {"get-device?state", JsonApi::ProcessJsonGetDeviceStateMessageWS},

            {"set-device", JsonApi::ProcessJsonSetDeviceMessageWS},
            {"set-device?state", JsonApi::ProcessJsonSetDeviceStateMessageWS},

            {"rem-device", JsonApi::ProcessJsonRemoveDeviceMessageWS},
        };

        bool WSSession::ProcessJsonApi(size_t id, const std::string& msg, rapidjson::Document& input,
                                       rapidjson::Document& output)
        {
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            rapidjson::Value logsJson = rapidjson::Value(rapidjson::kArrayType);

            ApiContext context = ApiContext(logsJson, allocator);

            // Call websocket message
            boost::unordered::unordered_map<std::string, WSMethod*>::const_iterator it = methodList.find(msg);
            if (it != methodList.end())
                it->second(user, input, output, context);
            else
            {
                context.Error("Invalid message method.");
                context.Error(ApiError::kError_InvalidArguments);
            }

            output.AddMember("logs", logsJson, allocator);
            output.AddMember("error", context.GetError(), allocator);
            output.AddMember("msgid", rapidjson::Value(id), allocator);
            output.AddMember(
                "msg", rapidjson::Value(context.GetError() == ApiError::kError_NoError ? "ack" : "nack", allocator),
                allocator);

            return true;
        }

        void WSSession::Send(rapidjson::Document& document)
        {
            Ref<rapidjson::StringBuffer> message = boost::make_shared<rapidjson::StringBuffer>();
            rapidjson::Writer<rapidjson::StringBuffer> writer = rapidjson::Writer<rapidjson::StringBuffer>(*message);
            document.Accept(writer);

            boost::lock_guard lock(queueMutex);

            messageQueue.push_back(message);

            if (messageQueue.size() == 1)
            {
                socket->async_write(
                    boost::asio::buffer(message->GetString(), message->GetSize()),
                    boost::asio::bind_executor(strand,
                                               boost::bind(&WSSession::OnWrite, shared_from_this(),
                                                           boost::placeholders::_1, boost::placeholders::_2, message)));
            }
        }
        void WSSession::Send(rapidjson::StringBuffer& buffer)
        {
            Ref<rapidjson::StringBuffer> message = boost::make_shared<rapidjson::StringBuffer>();
            message->Push(buffer.GetSize());
            memcpy((void*)message->GetString(), buffer.GetString(), buffer.GetSize());

            boost::lock_guard lock(queueMutex);

            messageQueue.push_back(message);

            if (messageQueue.size() == 1)
            {
                socket->async_write(
                    boost::asio::buffer(message->GetString(), message->GetSize()),
                    boost::asio::bind_executor(strand,
                                               boost::bind(&WSSession::OnWrite, shared_from_this(),
                                                           boost::placeholders::_1, boost::placeholders::_2, message)));
            }
        }
        void WSSession::Send(Ref<rapidjson::StringBuffer> buffer)
        {
            boost::lock_guard lock(queueMutex);

            messageQueue.push_back(buffer);

            if (messageQueue.size() == 1)
            {
                socket->async_write(boost::asio::buffer(buffer->GetString(), buffer->GetSize()),
                                    boost::asio::bind_executor(
                                        strand, boost::bind(&WSSession::OnWrite, shared_from_this(),
                                                            boost::placeholders::_1, boost::placeholders::_2, buffer)));
            }
        }

        void WSSession::OnWrite(boost::system::error_code error, size_t sentBytes, Ref<rapidjson::StringBuffer> message)
        {
            if (error)
                return;

            boost::lock_guard lock(queueMutex);

            messageQueue.erase(messageQueue.begin());

            if (messageQueue.size())
            {
                Ref<rapidjson::StringBuffer> message = messageQueue.front();

                socket->async_write(
                    boost::asio::buffer(message->GetString(), message->GetSize()),
                    boost::asio::bind_executor(strand,
                                               boost::bind(&WSSession::OnWrite, shared_from_this(),
                                                           boost::placeholders::_1, boost::placeholders::_2, message)));
            }
        }

        void WSSession::DoWSShutdown(boost::beast::websocket::close_code code, const char* reason)
        {
            // Shutdown
            socket->next_layer().next_layer().expires_after(std::chrono::seconds(6));
            socket->async_close(
                boost::beast::websocket::close_reason(code, reason),
                boost::asio::bind_executor(
                    strand, boost::bind(&WSSession::DoSSLShutdown, shared_from_this(), boost::placeholders::_1)));
        }
        void WSSession::DoSSLShutdown(boost::system::error_code error)
        {
            if (error)
                return;

            // Shutdown
            socket->next_layer().next_layer().expires_after(std::chrono::seconds(6));
            socket->next_layer().async_shutdown(boost::asio::bind_executor(
                strand, boost::bind(&WSSession::OnShutdown, shared_from_this(), boost::placeholders::_1)));
        }
        void WSSession::OnShutdown(boost::system::error_code error)
        {
            if (error)
                return;

            socket->next_layer().next_layer().close();
        }
    }
}