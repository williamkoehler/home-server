#include "Http.hpp"
#include "../Script.hpp"

namespace server
{
    namespace scripting
    {
        HttpController::HttpController(Ref<Script> script, CallbackMethod<> callback)
            : Controller(script), callback(callback)
        {
        }

        static const boost::beast::http::verb methods[] = {
            boost::beast::http::verb::get,     boost::beast::http::verb::get,     boost::beast::http::verb::head,
            boost::beast::http::verb::post,    boost::beast::http::verb::put,     boost::beast::http::verb::delete_,
            boost::beast::http::verb::connect, boost::beast::http::verb::options, boost::beast::http::verb::trace,
            boost::beast::http::verb::patch,
        };

        class HttpControllerImpl : public HttpController
        {
          private:
            boost::asio::ip::tcp::resolver resolver;
            boost::beast::tcp_stream stream;
            boost::beast::flat_buffer buffer;
            boost::beast::http::request<boost::beast::http::string_body> request;
            boost::beast::http::response<boost::beast::http::string_body> response;

            void OnResolve(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results)
            {
                if (!ec)
                {
                    // Set timeout
                    stream.expires_after(std::chrono::seconds(30));

                    // Connect to server
                    stream.async_connect(results,
                                         boost::beast::bind_front_handler(
                                             &HttpControllerImpl::OnConnect,
                                             boost::dynamic_pointer_cast<HttpControllerImpl>(shared_from_this())));
                }
                else
                {
                    // Invoke event
                    Ref<Script> r = script.lock();

                    if (r != nullptr)
                    {
                        // Set event args
                        statusCode = HttpStatusCode::kResolveError;
                        response.body().clear();

                        if (callback != nullptr)
                            (r.get()->*callback)(shared_from_this());
                    }
                }
            }
            void OnConnect(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type)
            {
                if (!ec)
                {
                    // Set timeout
                    stream.expires_after(std::chrono::seconds(10));

                    // Send data to server
                    boost::beast::http::async_write(
                        stream, request,
                        boost::beast::bind_front_handler(
                            &HttpControllerImpl::OnWrite,
                            boost::dynamic_pointer_cast<HttpControllerImpl>(shared_from_this())));
                }
                else
                {
                    // Invoke event
                    Ref<Script> r = script.lock();

                    if (r != nullptr)
                    {
                        // Set event args
                        statusCode = HttpStatusCode::kConnectError;
                        response.body().clear();

                        if (callback != nullptr)
                            (r.get()->*callback)(shared_from_this());
                    }
                }
            }
            void OnWrite(boost::beast::error_code ec, size_t bytes_transferred)
            {
                if (!ec)
                {
                    // Set timeout
                    stream.expires_after(std::chrono::seconds(10));

                    // Receive data from server
                    boost::beast::http::async_read(
                        stream, buffer, response,
                        boost::beast::bind_front_handler(
                            &HttpControllerImpl::OnRead,
                            boost::dynamic_pointer_cast<HttpControllerImpl>(shared_from_this())));
                }
                else
                {
                    // Invoke event
                    Ref<Script> r = script.lock();

                    if (r != nullptr)
                    {
                        // Set event args
                        statusCode = HttpStatusCode::kInternalError;
                        response.body().clear();

                        if (callback != nullptr)
                            (r.get()->*callback)(shared_from_this());
                    }
                }
            }
            void OnRead(boost::beast::error_code ec, size_t bytes_transferred)
            {
                if (!ec)
                {
                    // Invoke event
                    Ref<Script> r = script.lock();

                    if (r != nullptr)
                    {
                        // Invoke callback
                        if (callback != nullptr)
                            (r.get()->*callback)(shared_from_this());
                    }

                    // Gracefully close the socket
                    stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

                    // not_connected happens sometimes so don't bother reporting it
                    if (ec && ec != boost::beast::errc::not_connected)
                    {
                        // TODO: Error
                        LOG_CODE_MISSING("HTTP Shutdown error");
                    }

                    // Close connection gracefully
                }
                else
                {
                    // Invoke event
                    Ref<Script> r = script.lock();

                    if (r != nullptr)
                    {
                        // Set event args
                        statusCode = HttpStatusCode::kInternalError;
                        response.body().clear();

                        if (callback != nullptr)
                            (r.get()->*callback)(shared_from_this());
                    }
                }
            }

          public:
            HttpControllerImpl(Ref<Script> script, CallbackMethod<> callback)
                : HttpController(script, callback), resolver(Worker::GetInstance()->GetContext()),
                  stream(Worker::GetInstance()->GetContext())
            {
            }

            void Send(const std::string& host, uint16_t port, HttpMethod method, const std::string& target,
                      const std::string_view& content)
            {
                // Fill header
                request.version(11); // HTTP/1.1
                request.method(methods[(uint8_t)method]);
                request.target(target);

                // Set header arguments
                request.set(boost::beast::http::field::host, host);
                request.set(boost::beast::http::field::user_agent, "Home. Http Client");

                // Fill body
                request.body() = content;
                request.prepare_payload();

                // Look up the domain name
                resolver.async_resolve(host, std::to_string(port),
                                       boost::beast::bind_front_handler(
                                           &HttpControllerImpl::OnResolve,
                                           boost::dynamic_pointer_cast<HttpControllerImpl>(shared_from_this())));
            }

            virtual std::string_view GetContent() override
            {
                return response.body();
            }
        };

        bool Http::Send(Ref<Script> script, const std::string& host, uint16_t port, HttpMethod method,
                        const std::string& target, const std::string_view& content, CallbackMethod<> callback)
        {
            // Create new controller
            Ref<HttpControllerImpl> controller = boost::make_shared<HttpControllerImpl>(script, callback);

            if (controller != nullptr)
            {
                // Send http request
                controller->Send(host, port, method, target, content);

                return true;
            }

            return false;
        }

        class SslContext : public boost::asio::ssl::context
        {
          public:
            SslContext() : boost::asio::ssl::context(boost::asio::ssl::context::tlsv12_client)
            {
                set_options(boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::no_sslv2 |
                            boost::asio::ssl::context::no_sslv3 | boost::asio::ssl::context::tlsv13_client);
                set_default_verify_paths();
            }
        };

        static SslContext sslContext = SslContext();

        class HttpsControllerImpl : public HttpController
        {
          public:
            boost::asio::ip::tcp::resolver resolver;
            boost::beast::ssl_stream<boost::beast::tcp_stream> stream;
            boost::beast::flat_buffer buffer;
            boost::beast::http::request<boost::beast::http::string_body> request;
            boost::beast::http::response<boost::beast::http::string_body> response;

            void OnResolve(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results)
            {
                if (!ec)
                {
                    boost::beast::tcp_stream& stream2 = stream.next_layer();

                    // Set timeout
                    stream2.expires_after(std::chrono::seconds(30));

                    // Connect to server
                    stream2.async_connect(results,
                                          boost::beast::bind_front_handler(
                                              &HttpsControllerImpl::OnConnect,
                                              boost::dynamic_pointer_cast<HttpsControllerImpl>(shared_from_this())));
                }
                else
                {
                    // Invoke event
                    Ref<Script> r = script.lock();

                    if (r != nullptr)
                    {
                        // Set event args
                        statusCode = HttpStatusCode::kResolveError;
                        response.body().assign(ec.message());

                        if (callback != nullptr)
                            (r.get()->*callback)(shared_from_this());
                    }
                }
            }
            void OnConnect(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type)
            {
                if (!ec)
                {
                    boost::beast::tcp_stream& stream2 = stream.next_layer();

                    // Set timeout
                    stream2.expires_after(std::chrono::seconds(30));

                    // Connect to server
                    stream.async_handshake(boost::asio::ssl::stream_base::client,
                                           boost::beast::bind_front_handler(
                                               &HttpsControllerImpl::OnHandshake,
                                               boost::dynamic_pointer_cast<HttpsControllerImpl>(shared_from_this())));
                }
                else
                {
                    // Invoke event
                    Ref<Script> r = script.lock();

                    if (r != nullptr)
                    {
                        // Set event args
                        statusCode = HttpStatusCode::kResolveError;
                        response.body().assign(ec.message());

                        if (callback != nullptr)
                            (r.get()->*callback)(shared_from_this());
                    }
                }
            }
            void OnHandshake(boost::beast::error_code ec)
            {
                if (!ec)
                {
                    boost::beast::tcp_stream& stream2 = stream.next_layer();

                    // Set timeout
                    stream2.expires_after(std::chrono::seconds(10));

                    // Send data to server
                    boost::beast::http::async_write(
                        stream, request,
                        boost::beast::bind_front_handler(
                            &HttpsControllerImpl::OnWrite,
                            boost::dynamic_pointer_cast<HttpsControllerImpl>(shared_from_this())));
                }
                else
                {
                    // Invoke event
                    Ref<Script> r = script.lock();

                    if (r != nullptr)
                    {
                        // Set event args
                        statusCode = HttpStatusCode::kHandshakeError;
                        response.body().assign(ec.message());

                        if (callback != nullptr)
                            (r.get()->*callback)(shared_from_this());
                    }
                }
            }
            void OnWrite(boost::beast::error_code ec, size_t bytes_transferred)
            {
                if (!ec)
                {
                    boost::beast::tcp_stream& stream2 = stream.next_layer();

                    // Set timeout
                    stream2.expires_after(std::chrono::seconds(10));

                    // Receive data from server
                    boost::beast::http::async_read(
                        stream, buffer, response,
                        boost::beast::bind_front_handler(
                            &HttpsControllerImpl::OnRead,
                            boost::dynamic_pointer_cast<HttpsControllerImpl>(shared_from_this())));
                }
                else
                {
                    // Invoke event
                    Ref<Script> r = script.lock();

                    if (r != nullptr)
                    {
                        // Set event args
                        statusCode = HttpStatusCode::kInternalError;
                        response.body().assign(ec.message());

                        if (callback != nullptr)
                            (r.get()->*callback)(shared_from_this());
                    }
                }
            }
            void OnRead(boost::beast::error_code ec, size_t bytes_transferred)
            {
                if (!ec)
                {
                    // Invoke event
                    Ref<Script> r = script.lock();

                    if (r != nullptr)
                    {
                        // Invoke callback
                        if (callback != nullptr)
                            (r.get()->*callback)(shared_from_this());
                    }

                    // Gracefully close the socket
                    stream.next_layer().socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

                    // not_connected happens sometimes so don't bother reporting it
                    if (ec && ec != boost::beast::errc::not_connected)
                    {
                        // TODO: Error
                        LOG_CODE_MISSING("HTTP Shutdown error");
                    }

                    // Close connection gracefully
                }
                else
                {
                    // Invoke event
                    Ref<Script> r = script.lock();

                    if (r != nullptr)
                    {
                        // Set event args
                        statusCode = HttpStatusCode::kInternalError;
                        response.body().assign(ec.message());

                        if (callback != nullptr)
                            (r.get()->*callback)(shared_from_this());
                    }
                }
            }

          public:
            HttpsControllerImpl(Ref<Script> script, CallbackMethod<> callback)
                : HttpController(script, callback),
                  resolver(Worker::GetInstance()->GetContext()),
                  stream(Worker::GetInstance()->GetContext(), sslContext)
            {
            }

            void Send(const std::string& host, uint16_t port, HttpMethod method, const std::string& target,
                      const std::string_view& content)
            {
                // Fill header
                request.version(11); // HTTP/1.1
                request.method(methods[(uint8_t)method]);
                request.target(target);

                // Set header arguments
                request.set(boost::beast::http::field::host, host);
                request.set(boost::beast::http::field::user_agent, "Home. Http Client");

                // Fill body
                request.body() = content;
                request.prepare_payload();

                // Look up the domain name
                resolver.async_resolve(host, std::to_string(port),
                                       boost::beast::bind_front_handler(
                                           &HttpsControllerImpl::OnResolve,
                                           boost::dynamic_pointer_cast<HttpsControllerImpl>(shared_from_this())));
            }

            virtual std::string_view GetContent() override
            {
                return response.body();
            }
        };

        bool Https::Send(Ref<Script> script, const std::string& host, uint16_t port, HttpMethod method,
                         const std::string& target, const std::string_view& content, CallbackMethod<> callback)
        {
            // Create new controller
            Ref<HttpsControllerImpl> controller = boost::make_shared<HttpsControllerImpl>(script, callback);

            if (controller != nullptr)
            {
                // Send http request
                controller->Send(host, port, method, target, content);

                return true;
            }

            return false;
        }
    }
}