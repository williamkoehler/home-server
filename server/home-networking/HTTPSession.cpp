#include "HTTPSession.hpp"
#include "WSSession.hpp"
#include "json/JsonApi.hpp"
#include <cppcodec/base64_rfc4648.hpp>
#include <home-users/UserManager.hpp>

namespace server
{
    namespace networking
    {
        HTTPSession::HTTPSession(Ref<tcp_socket_t> socket) : strand(socket->get_executor()), socket(socket)
        {
        }
        HTTPSession::~HTTPSession()
        {
        }

        void HTTPSession::Run()
        {
            socket->expires_after(std::chrono::seconds(12));
            boost::beast::http::async_read(
                *socket, buffer, request,
                boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnRead, shared_from_this(),
                                                               boost::placeholders::_1, boost::placeholders::_2)));
        }

        void HTTPSession::OnRead(boost::system::error_code error, size_t size)
        {
            if (error)
                return;

            try
            {

                std::string_view target = std::string_view(request.target().data(), request.target().size());

                if (strncmp(target.data(), "/res/", 5) == 0)
                {
                    Ref<users::User> user = Authenticate();
                    if (user == nullptr)
                    {
                        WriteError("Invalid authentication token.");
                        return;
                    }

                    target.remove_prefix(5); // Remove /api/

                    // Create response
                    boost::shared_ptr<boost::beast::http::response<boost::beast::http::buffer_body>> response =
                        boost::make_shared<boost::beast::http::response<boost::beast::http::buffer_body>>();
                    response->version(request.version());
                    response->set(boost::beast::http::field::server, "HomeAutomation Server");

                    // Resource
                    std::string contentType = "text/plain";

                    responseBuffer.Clear();

                    if (JsonApi::ProcessResApiCallHTTP(request.method(), target, user, std::string_view(request.body()),
                                                       responseBuffer, contentType))
                    {
                        response->result(boost::beast::http::status::ok);
                        response->keep_alive(request.keep_alive());
                    }
                    else
                    {
                        response->result(boost::beast::http::status::bad_request);
                        response->keep_alive(false);
                    }

                    if (responseBuffer.GetSize() == 0)
                    {
                        // A response cannot be empty
                        response->set(boost::beast::http::field::content_type, "test/plain");
                        memcpy(responseBuffer.Push(10), "NO CONTENT", 10);
                    }
                    else
                        response->set(boost::beast::http::field::content_type, contentType);

                    boost::beast::http::buffer_body::value_type& buf = response->body();
                    buf.data = (void*)responseBuffer.GetString();
                    buf.size = responseBuffer.GetSize();
                    buf.more = false;

                    response->prepare_payload();

                    buffer.consume(size);

                    // Send response
                    boost::beast::http::async_write(
                        *socket, *response,
                        boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnWriteBuffer, shared_from_this(),
                                                                       boost::placeholders::_1, boost::placeholders::_2,
                                                                       response)));
                }
                else if (strncmp(target.data(), "/auth", 5) == 0)
                {
                    Ref<users::User> user = Authenticate();
                    if (user == nullptr)
                    {
                        WriteError("Invalid authentication token.");
                        return;
                    }

                    Ref<users::UserManager> userManager = users::UserManager::GetInstance();
                    assert(userManager != nullptr);

                    std::string token = userManager->GenerateJWTToken(user);

                    // Create response
                    boost::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>> response =
                        boost::make_shared<boost::beast::http::response<boost::beast::http::string_body>>();
                    response->result(boost::beast::http::status::ok);
                    response->version(request.version());
                    response->keep_alive(request.keep_alive());
                    response->set(boost::beast::http::field::server, "HomeAutomation Server");
                    response->set(boost::beast::http::field::content_type, "application/json");
                    std::string& body = response->body();
                    body.reserve(12 + token.size());
                    body += "{\"token\":\"";
                    body += token;
                    body += "\"}";

                    response->prepare_payload();

                    // Send response
                    boost::beast::http::async_write(
                        *socket, *response,
                        boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnWriteString, shared_from_this(),
                                                                       boost::placeholders::_1, boost::placeholders::_2,
                                                                       response)));
                }
                else if (strncmp(target.data(), "/ws", 3) == 0)
                {
                    if (boost::beast::websocket::is_upgrade(request))
                    {
                        Ref<users::User> user = Authenticate();
                        if (user == nullptr)
                        {
                            WriteError("Invalid authentication token.");
                            return;
                        }

                        socket->expires_never();

                        Ref<WSSession> ws = boost::make_shared<WSSession>(socket, user);
                        ws->Run(request);

                        buffer.consume(size);
                    }
                    else
                    {
                        WriteError("Invalid websocket upgrade. Please call /help for more details.");
                        return;
                    }
                }
                else if (strncmp(target.data(), "/ping", 5) == 0)
                {
                    // Create response
                    boost::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>> response =
                        boost::make_shared<boost::beast::http::response<boost::beast::http::string_body>>();
                    response->result(boost::beast::http::status::ok);
                    response->version(request.version());
                    response->keep_alive(request.keep_alive());
                    response->set(boost::beast::http::field::server, "HomeAutomation Server");
                    response->set(boost::beast::http::field::content_type, "application/json");
                    std::string& body = response->body();
                    body.reserve(38);
                    body += R"({"pong":"57494c4c49414d4bd6484c4552"})";
                    response->prepare_payload();

                    // Send response
                    boost::beast::http::async_write(
                        *socket, *response,
                        boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnWriteString, shared_from_this(),
                                                                       boost::placeholders::_1, boost::placeholders::_2,
                                                                       response)));
                }
                else if (strncmp(target.data(), "/help", 5) == 0)
                {
                    // Create response
                    boost::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>> response =
                        boost::make_shared<boost::beast::http::response<boost::beast::http::string_body>>();
                    response->result(boost::beast::http::status::ok);
                    response->version(request.version());
                    response->keep_alive(request.keep_alive());
                    response->set(boost::beast::http::field::server, "HomeAutomation Server");
                    response->set(boost::beast::http::field::content_type, "test/plain");

                    // Write help log
                    {
                        std::stringstream ss(response->body());

                        ss << "You called for help!\nWell currently nobody can help you...";
                    }

                    response->prepare_payload();

                    // Send response
                    boost::beast::http::async_write(
                        *socket, *response,
                        boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnWriteString, shared_from_this(),
                                                                       boost::placeholders::_1, boost::placeholders::_2,
                                                                       response)));
                }
                else
                {
                    buffer.consume(size);

                    WriteError("Invalid method.");
                }
            }
            catch (std::exception e)
            {
                LOG_ERROR("Oops... Internal error :\n{0}", e.what());

                buffer.clear();

                WriteError("Internal error.");
            }
        }
        Ref<users::User> HTTPSession::Authenticate()
        {
            boost::beast::http::fields::iterator it = request.find(boost::beast::http::field::authorization);
            if (it == request.end())
            {
                WriteError("Missing authorization field. Please call /api/help for more details.");
                return nullptr;
            }

            Ref<users::UserManager> userManager = users::UserManager::GetInstance();
            assert(userManager != nullptr);

            boost::beast::string_view authorization = it->value();
            if (strncmp(authorization.data(), "Bearer ", 7) == 0)
            { // Authenticate using bearer token
                authorization.remove_prefix(7);

                identifier_t userID = userManager->VerifyJWTToken(authorization.to_string());
                return userManager->GetUser(userID);
            }
            else if (strncmp(authorization.data(), "Basic ", 6) == 0)
            { // Authenticate using basic token
                authorization.remove_prefix(6);

                try
                {
                    std::vector<uint8_t> decoded = cppcodec::base64_rfc4648::decode<std::vector<uint8_t>>(
                        authorization.data(), authorization.size());

                    std::string_view basic = std::string_view((const char*)decoded.data(), decoded.size());

                    size_t seperator = basic.find(':');
                    if (seperator == std::string_view::npos)
                    {
                        WriteError("Invalid authorization field. Please call /api/help for more details.");
                        return nullptr;
                    }

                    std::string_view name = basic.substr(0, seperator);
                    std::string_view password = basic.substr(seperator + 1);

                    return userManager->Authenticate(name, password);
                }
                catch (std::exception)
                {
                }
            }

            // Write error
            WriteError("Invalid authorization field. Please call /api/help for more details.");
            return nullptr;
        }
        void HTTPSession::WriteError(const char* error)
        {
            boost::shared_ptr<boost::beast::http::response<boost::beast::http::buffer_body>> response =
                boost::make_shared<boost::beast::http::response<boost::beast::http::buffer_body>>();
            response->result(boost::beast::http::status::bad_request);
            response->version(request.version());
            response->keep_alive(false);
            response->set(boost::beast::http::field::server, "HomeAutomation Server");
            response->set(boost::beast::http::field::content_type, "application/json");

            rapidjson::Document responseDocument = rapidjson::Document(rapidjson::kObjectType);

            JsonApi::BuildJsonErrorMessageHTTP(error, responseDocument);

            responseBuffer.Clear();
            rapidjson::Writer<rapidjson::StringBuffer> writer =
                rapidjson::Writer<rapidjson::StringBuffer>(responseBuffer);
            responseDocument.Accept(writer);

            boost::beast::http::buffer_body::value_type& buf = response->body();
            buf.data = (void*)responseBuffer.GetString();
            buf.size = responseBuffer.GetSize();
            buf.more = false;

            response->prepare_payload();

            boost::beast::http::async_write(
                *socket, *response,
                boost::asio::bind_executor(strand,
                                           boost::bind(&HTTPSession::OnWriteBuffer, shared_from_this(),
                                                       boost::placeholders::_1, boost::placeholders::_2, response)));
        }

        void HTTPSession::OnWrite(
            boost::system::error_code error, size_t size,
            boost::shared_ptr<boost::beast::http::response<boost::beast::http::empty_body>> response)
        {
            if (error)
                return;

            if (response->keep_alive())
            {
                request = {};
                socket->expires_after(std::chrono::seconds(12));
                boost::beast::http::async_read(
                    *socket, buffer, request,
                    boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnRead, shared_from_this(),
                                                                   boost::placeholders::_1, boost::placeholders::_2)));
            }
            else
            {
                // // Shutdown
                // socket->expires_after(std::chrono::seconds(6));
                // socket->async_shutdown(boost::asio::bind_executor(
                //     strand, boost::bind(&HTTPSession::OnShutdown, shared_from_this(), boost::placeholders::_1)));
                socket->close();
            }
        }
        void HTTPSession::OnWriteString(
            boost::system::error_code error, size_t size,
            boost::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>> response)
        {
            if (error)
                return;

            if (response->keep_alive())
            {
                request = {};
                socket->expires_after(std::chrono::seconds(12));
                boost::beast::http::async_read(
                    *socket, buffer, request,
                    boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnRead, shared_from_this(),
                                                                   boost::placeholders::_1, boost::placeholders::_2)));
            }
            else
            {
                // // Shutdown
                // socket->next_layer().expires_after(std::chrono::seconds(6));
                // socket->async_shutdown(boost::asio::bind_executor(
                //     strand, boost::bind(&HTTPSession::OnShutdown, shared_from_this(), boost::placeholders::_1)));
                socket->close();
            }
        }
        void HTTPSession::OnWriteBuffer(
            boost::system::error_code error, size_t size,
            boost::shared_ptr<boost::beast::http::response<boost::beast::http::buffer_body>> response)
        {
            if (error)
                return;

            if (response->keep_alive())
            {
                request = {};
                socket->expires_after(std::chrono::seconds(12));
                boost::beast::http::async_read(
                    *socket, buffer, request,
                    boost::asio::bind_executor(strand, boost::bind(&HTTPSession::OnRead, shared_from_this(),
                                                                   boost::placeholders::_1, boost::placeholders::_2)));
            }
            else
            {
                // // Shutdown
                // socket->next_layer().expires_after(std::chrono::seconds(6));
                // socket->async_shutdown(boost::asio::bind_executor(
                //     strand, boost::bind(&HTTPSession::OnShutdown, shared_from_this(), boost::placeholders::_1)));
                socket->close();
            }
        }

        void HTTPSession::OnShutdown(boost::system::error_code error)
        {
            if (error)
                return;

            // // At this point the session will close itself
            // socket->next_layer().close();
        }
    }
}