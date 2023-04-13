#include "ApiSession.hpp"
#include "NetworkManager.hpp"
#include "json/JsonApi.hpp"

namespace server
{
    namespace networking
    {
        typedef void(WSMethod)(const Ref<users::User>&, const ApiRequestMessage&, ApiResponseMessage&,
                               const Ref<ApiSession>&);
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

            // Entity
            {"add-entity", JsonApi::ProcessJsonAddEntityMessageWS},

            {"inv-entiy", JsonApi::ProcessJsonInvokeDeviceMethodMessageWS},

            {"sub-to-entity-state", JsonApi::ProcessJsonSubscribeToEntityStateMessageWS},
            {"unsub-from-entity-state", JsonApi::ProcessJsonUnsubscribeFromEntityStateMessageWS},

            {"get-entity", JsonApi::ProcessJsonGetEntityMessageWS},
            {"get-entity-state", JsonApi::ProcessJsonGetEntityStateMessageWS},

            {"set-entity", JsonApi::ProcessJsonSetEntityMessageWS},
            {"set-entity-state", JsonApi::ProcessJsonSetEntityStateMessageWS},

            {"rem-entity", JsonApi::ProcessJsonRemoveEntityMessageWS},
        };

        ApiSessionImpl::ApiSessionImpl(const Ref<tcp_socket_t>& socket, const Ref<users::User>& user)
            : strand(socket->get_executor()), user(user), socket(boost::make_shared<websocket_t>(std::move(*socket)))
        {
        }
        ApiSessionImpl::~ApiSessionImpl()
        {
        }

        void ApiSessionImpl::Run(boost::beast::http::request<boost::beast::http::string_body>& request)
        {
            socket->next_layer().expires_after(std::chrono::seconds(12));
            socket->set_option(boost::beast::websocket::stream_base::decorator(
                [](boost::beast::websocket::response_type& response) -> void
                { response.set(boost::beast::http::field::server, "HomeAutomation Server WebSocket"); }));
            socket->async_accept(
                request, boost::asio::bind_executor(strand, boost::bind(&ApiSessionImpl::OnAccept, shared_from_this(),
                                                                        boost::placeholders::_1)));
        }

        void ApiSessionImpl::OnAccept(const boost::system::error_code& ec)
        {
            if (ec)
                return;

            socket->next_layer().expires_never();

            // Add ws to publish list
            {
                Ref<NetworkManager> networkManager = NetworkManager::GetInstance();
                networkManager->sessionList.push_back(shared_from_this());
            }

            socket->text(true);
            socket->async_read(buffer, boost::asio::bind_executor(
                                           strand, boost::bind(&ApiSessionImpl::OnRead, shared_from_this(),
                                                               boost::placeholders::_1, boost::placeholders::_2)));
        }

        void ApiSessionImpl::OnRead(const boost::system::error_code& ec, size_t receivedBytes)
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
                                               strand, boost::bind(&ApiSessionImpl::OnRead, shared_from_this(),
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
            boost::unordered::unordered_map<std::string, WSMethod*>::const_iterator it =
                methodList.find(request.GetType());
            if (it != methodList.end())
                it->second(user, request, response, shared_from_this());
            else
                response.SetErrorCode(kApiErrorCode_InvalidArguments);

            Send(id, response);

            // Wait for data
            socket->async_read(buffer, boost::asio::bind_executor(
                                           strand, boost::bind(&ApiSessionImpl::OnRead, shared_from_this(),
                                                               boost::placeholders::_1, boost::placeholders::_2)));
        }

        void ApiSessionImpl::Send(size_t id, const ApiMessage& message)
        {
            Ref<rapidjson::StringBuffer> buffer = boost::make_shared<rapidjson::StringBuffer>();
            if (buffer != nullptr)
            {
                // Build message
                {
                    rapidjson::Writer<rapidjson::StringBuffer> writer =
                        rapidjson::Writer<rapidjson::StringBuffer>(*buffer);
                    message.Build(id, writer);
                }

                messageQueue.push_back(buffer);

                if (messageQueue.size() == 1)
                {
                    socket->async_write(
                        boost::asio::buffer(buffer->GetString(), buffer->GetSize()),
                        boost::asio::bind_executor(strand, boost::bind(&ApiSessionImpl::OnWrite, shared_from_this(),
                                                                       boost::placeholders::_1, boost::placeholders::_2,
                                                                       buffer)));
                }
            }
            else
            {
                LOG_ERROR("Failed to create string buffer.");
            }
        }

        void ApiSessionImpl::Send(const ApiMessage& message)
        {
            Ref<rapidjson::StringBuffer> buffer = boost::make_shared<rapidjson::StringBuffer>();
            if (buffer != nullptr)
            {
                // Build message
                {
                    rapidjson::Writer<rapidjson::StringBuffer> writer =
                        rapidjson::Writer<rapidjson::StringBuffer>(*buffer);
                    message.Build(0, writer);
                }

                messageQueue.push_back(buffer);

                if (messageQueue.size() == 1)
                {
                    socket->async_write(
                        boost::asio::buffer(buffer->GetString(), buffer->GetSize()),
                        boost::asio::bind_executor(strand, boost::bind(&ApiSessionImpl::OnWrite, shared_from_this(),
                                                                       boost::placeholders::_1, boost::placeholders::_2,
                                                                       buffer)));
                }
            }
            else
            {
                LOG_ERROR("Failed to create string buffer.");
            }
        }

        void ApiSessionImpl::Send(const Ref<rapidjson::StringBuffer>& buffer)
        {
            if (buffer != nullptr)
            {
                messageQueue.push_back(buffer);

                if (messageQueue.size() == 1)
                {
                    socket->async_write(
                        boost::asio::buffer(buffer->GetString(), buffer->GetSize()),
                        boost::asio::bind_executor(strand, boost::bind(&ApiSessionImpl::OnWrite, shared_from_this(),
                                                                       boost::placeholders::_1, boost::placeholders::_2,
                                                                       buffer)));
                }
            }
        }

        void ApiSessionImpl::OnWrite(const boost::system::error_code& ec, size_t sentBytes,
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
                                               boost::bind(&ApiSessionImpl::OnWrite, shared_from_this(),
                                                           boost::placeholders::_1, boost::placeholders::_2, message)));
            }
        }

        void ApiSessionImpl::DoWSShutdown(boost::beast::websocket::close_code code, const char* reason)
        {
            // Shutdown
            socket->next_layer().expires_after(std::chrono::seconds(6));
            socket->async_close(
                boost::beast::websocket::close_reason(code, reason),
                boost::asio::bind_executor(
                    strand, boost::bind(&ApiSessionImpl::OnShutdown, shared_from_this(), boost::placeholders::_1)));
        }
        void ApiSessionImpl::DoSSLShutdown(const boost::system::error_code& ec)
        {
            if (ec)
                return;

            // // Shutdown
            // socket->next_layer().expires_after(std::chrono::seconds(6));
            // socket->next_layer().async_shutdown(boost::asio::bind_executor(
            //     strand, boost::bind(&WSSession::OnShutdown, shared_from_this(), boost::placeholders::_1)));
        }
        void ApiSessionImpl::OnShutdown(const boost::system::error_code& ec)
        {
            if (ec)
                return;

            socket->next_layer().close();
        }
    }
}