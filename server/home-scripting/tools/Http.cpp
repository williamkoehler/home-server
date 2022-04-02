#include "Http.hpp"
#include "../Script.hpp"

namespace server
{
    namespace scripting
    {
        static const boost::beast::http::verb methods[] = {
            boost::beast::http::verb::get,     boost::beast::http::verb::get,     boost::beast::http::verb::head,
            boost::beast::http::verb::post,    boost::beast::http::verb::put,     boost::beast::http::verb::delete_,
            boost::beast::http::verb::connect, boost::beast::http::verb::options, boost::beast::http::verb::trace,
            boost::beast::http::verb::patch,
        };

        class HttpConnectionImpl : public HttpConnection
        {
          private:
            WeakRef<Script> script;

            boost::asio::ip::tcp::resolver resolver;
            boost::beast::tcp_stream stream;
            boost::beast::flat_buffer buffer;
            boost::beast::http::request<boost::beast::http::string_body> request;
            boost::beast::http::response<boost::beast::http::string_body> response;

            std::string callback;

            void OnResolve(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results)
            {
                if (!ec)
                {
                    // Set timeout
                    stream.expires_after(std::chrono::seconds(30));

                    // Connect to server
                    stream.async_connect(results,
                                         boost::beast::bind_front_handler(
                                             &HttpConnectionImpl::OnConnect,
                                             boost::dynamic_pointer_cast<HttpConnectionImpl>(shared_from_this())));
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

                        r->Invoke(callback, shared_from_this());
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
                            &HttpConnectionImpl::OnWrite,
                            boost::dynamic_pointer_cast<HttpConnectionImpl>(shared_from_this())));
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

                        r->Invoke(callback, shared_from_this());
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
                            &HttpConnectionImpl::OnRead,
                            boost::dynamic_pointer_cast<HttpConnectionImpl>(shared_from_this())));
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

                        r->Invoke(callback, shared_from_this());
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
                        r->Invoke(callback, shared_from_this());
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

                        r->Invoke(callback, shared_from_this());
                    }
                }
            }

          public:
            HttpConnectionImpl(Ref<Script> script, const std::string& callback)
                : script(script), resolver(boost::asio::make_strand(script->GetWorker()->GetContext())),
                  stream(boost::asio::make_strand(script->GetWorker()->GetContext())), callback(callback)
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
                                           &HttpConnectionImpl::OnResolve,
                                           boost::dynamic_pointer_cast<HttpConnectionImpl>(shared_from_this())));
            }

            virtual std::string_view GetContent() override
            {
                return response.body();
            }
        };

        bool Http::Send(Ref<Script> script, const std::string& callback, const std::string& host, uint16_t port,
                        HttpMethod method, const std::string& target, const std::string_view& content)
        {
            // Create new session
            Ref<HttpConnectionImpl> session = boost::make_shared<HttpConnectionImpl>(script, callback);

            if (session != nullptr)
            {
                // Send http request
                session->Send(host, port, method, target, content);

                return true;
            }

            return false;
        }

        class SslContext : public boost::asio::ssl::context
        {
          public:
            SslContext() : boost::asio::ssl::context(boost::asio::ssl::context::tlsv12_client)
            {
                set_verify_mode(boost::asio::ssl::verify_none);
            }
        };

        class HttpsConnectionImpl : public HttpConnection
        {
          public:
            WeakRef<Script> script;

            boost::asio::ip::tcp::resolver resolver;
            SslContext sslContext;
            boost::beast::ssl_stream<boost::beast::tcp_stream> stream;
            boost::beast::flat_buffer buffer;
            boost::beast::http::request<boost::beast::http::string_body> request;
            boost::beast::http::response<boost::beast::http::string_body> response;

            std::string callback;

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
                                              &HttpsConnectionImpl::OnConnect,
                                              boost::dynamic_pointer_cast<HttpsConnectionImpl>(shared_from_this())));
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

                        r->Invoke(callback, shared_from_this());
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
                                               &HttpsConnectionImpl::OnHandshake,
                                               boost::dynamic_pointer_cast<HttpsConnectionImpl>(shared_from_this())));
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

                        r->Invoke(callback, shared_from_this());
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
                            &HttpsConnectionImpl::OnWrite,
                            boost::dynamic_pointer_cast<HttpsConnectionImpl>(shared_from_this())));
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

                        r->Invoke(callback, shared_from_this());
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
                            &HttpsConnectionImpl::OnRead,
                            boost::dynamic_pointer_cast<HttpsConnectionImpl>(shared_from_this())));
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

                        r->Invoke(callback, shared_from_this());
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
                        r->Invoke(callback, shared_from_this());
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

                        r->Invoke(callback, shared_from_this());
                    }
                }
            }

          public:
            HttpsConnectionImpl(Ref<Script> script, const std::string& callback)
                : script(script), resolver(boost::asio::make_strand(script->GetWorker()->GetContext())), sslContext(),
                  stream(boost::asio::make_strand(script->GetWorker()->GetContext()), sslContext), callback(callback)
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
                                           &HttpsConnectionImpl::OnResolve,
                                           boost::dynamic_pointer_cast<HttpsConnectionImpl>(shared_from_this())));
            }

            virtual std::string_view GetContent() override
            {
                return response.body();
            }
        };

        bool Https::Send(Ref<Script> script, const std::string& callback, const std::string& host, uint16_t port,
                         HttpMethod method, const std::string& target, const std::string_view& content)
        {
            // Create new session
            Ref<HttpsConnectionImpl> session = boost::make_shared<HttpsConnectionImpl>(script, callback);

            if (session != nullptr)
            {
                // Send http request
                session->Send(host, port, method, target, content);

                return true;
            }

            return false;
        }
    }
}