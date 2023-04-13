#include "Http.hpp"
#include <home-scripting/Script.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

namespace server
{
    namespace scripting
    {
        namespace native
        {
            HttpController::HttpController(const Ref<NativeScriptImpl>& script,
                                           UniqueRef<HttpController::CallbackMethod> callbackMethod)
                : Controller(script), callbackMethod(std::move(callbackMethod))
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

                void OnResolve(const boost::beast::error_code& ec,
                               const boost::asio::ip::tcp::resolver::results_type& results)
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
                        Ref<NativeScriptImpl> r = script.lock();

                        if (r != nullptr)
                        {
                            // Set event args
                            statusCode = HttpStatusCode::kResolveError;
                            response.body().clear();

                            if (callbackMethod != nullptr)
                            {
                                callbackMethod->Invoke(r.get(),
                                                       boost::dynamic_pointer_cast<HttpController>(shared_from_this()));
                            }
                        }
                    }
                }
                void OnConnect(const boost::beast::error_code& ec,
                               const boost::asio::ip::tcp::resolver::results_type::endpoint_type&)
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
                        Ref<NativeScriptImpl> r = script.lock();

                        if (r != nullptr)
                        {
                            // Set event args
                            statusCode = HttpStatusCode::kConnectError;
                            response.body().clear();

                            if (callbackMethod != nullptr)
                            {
                                callbackMethod->Invoke(r.get(),
                                                       boost::dynamic_pointer_cast<HttpController>(shared_from_this()));
                            }
                        }
                    }
                }
                void OnWrite(const boost::beast::error_code& ec, size_t bytes_transferred)
                {
                    (void)bytes_transferred;

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
                        Ref<NativeScriptImpl> r = script.lock();

                        if (r != nullptr)
                        {
                            // Set event args
                            statusCode = HttpStatusCode::kInternalError;
                            response.body().clear();

                            if (callbackMethod != nullptr)
                            {
                                callbackMethod->Invoke(r.get(),
                                                       boost::dynamic_pointer_cast<HttpController>(shared_from_this()));
                            }
                        }
                    }
                }
                void OnRead(const boost::beast::error_code& ec, size_t bytes_transferred)
                {
                    (void)bytes_transferred;

                    if (!ec)
                    {
                        // Set event args
                        statusCode = (HttpStatusCode)response.result_int();

                        // Invoke event
                        Ref<NativeScriptImpl> r = script.lock();

                        if (r != nullptr)
                        {
                            // Invoke callback
                            if (callbackMethod != nullptr)
                            {
                                callbackMethod->Invoke(r.get(),
                                                       boost::dynamic_pointer_cast<HttpController>(shared_from_this()));
                            }
                        }

                        // Gracefully close the socket
                        boost::beast::error_code ec2;
                        stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec2);

                        // not_connected happens sometimes so don't bother reporting it
                        if (ec2 && ec2 != boost::beast::errc::not_connected)
                        {
                            // TODO: Error
                            LOG_CODE_MISSING("HTTP Shutdown error");
                        }

                        // Close connection gracefully
                    }
                    else
                    {
                        // Invoke event
                        Ref<NativeScriptImpl> r = script.lock();

                        if (r != nullptr)
                        {
                            // Set event args
                            statusCode = HttpStatusCode::kInternalError;
                            response.body().clear();

                            if (callbackMethod != nullptr)
                            {
                                callbackMethod->Invoke(r.get(),
                                                       boost::dynamic_pointer_cast<HttpController>(shared_from_this()));
                            }
                        }
                    }
                }

              public:
                HttpControllerImpl(const Ref<NativeScriptImpl>& script,
                                   UniqueRef<HttpController::CallbackMethod> methodCallback)
                    : HttpController(script, std::move(methodCallback)), resolver(Worker::GetInstance()->GetContext()),
                      stream(Worker::GetInstance()->GetContext())
                {
                }

                void Send(const std::string& host, uint16_t port, HttpMethod method, const std::string& target,
                          const std::string_view& content, const std::string_view& contentType)
                {
                    // Fill header
                    request.version(11); // HTTP/1.1
                    request.method(methods[(uint8_t)method]);
                    request.target(target);

                    // Set header arguments
                    request.set(boost::beast::http::field::host, host);
                    request.set(boost::beast::http::field::user_agent, "Home. Http Client");
                    request.set(boost::beast::http::field::content_type, contentType.data());

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

            bool Http::Send(const Ref<NativeScriptImpl>& script, const std::string& host, uint16_t port,
                            HttpMethod method, const std::string& target, const std::string_view& content,
                            const std::string_view& contentType,
                            UniqueRef<HttpController::CallbackMethod> methodCallback)
            {
                // Create new controller
                Ref<HttpControllerImpl> controller =
                    boost::make_shared<HttpControllerImpl>(script, std::move(methodCallback));

                if (controller != nullptr)
                {
                    // Send http request
                    controller->Send(host, port, method, target, content, contentType);

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

                void OnResolve(const boost::beast::error_code& ec,
                               const boost::asio::ip::tcp::resolver::results_type& results)
                {
                    if (!ec)
                    {
                        boost::beast::tcp_stream& stream2 = stream.next_layer();

                        // Set timeout
                        stream2.expires_after(std::chrono::seconds(30));

                        // Connect to server
                        stream2.async_connect(
                            results, boost::beast::bind_front_handler(
                                         &HttpsControllerImpl::OnConnect,
                                         boost::dynamic_pointer_cast<HttpsControllerImpl>(shared_from_this())));
                    }
                    else
                    {
                        // Invoke event
                        Ref<NativeScriptImpl> r = script.lock();

                        if (r != nullptr)
                        {
                            // Set event args
                            statusCode = HttpStatusCode::kResolveError;
                            response.body().assign(ec.message());

                            if (callbackMethod != nullptr)
                            {
                                callbackMethod->Invoke(r.get(),
                                                       boost::dynamic_pointer_cast<HttpController>(shared_from_this()));
                            }
                        }
                    }
                }
                void OnConnect(const boost::beast::error_code& ec,
                               const boost::asio::ip::tcp::resolver::results_type::endpoint_type&)
                {
                    if (!ec)
                    {
                        boost::beast::tcp_stream& stream2 = stream.next_layer();

                        // Set timeout
                        stream2.expires_after(std::chrono::seconds(30));

                        // Connect to server
                        stream.async_handshake(
                            boost::asio::ssl::stream_base::client,
                            boost::beast::bind_front_handler(
                                &HttpsControllerImpl::OnHandshake,
                                boost::dynamic_pointer_cast<HttpsControllerImpl>(shared_from_this())));
                    }
                    else
                    {
                        // Invoke event
                        Ref<NativeScriptImpl> r = script.lock();

                        if (r != nullptr)
                        {
                            // Set event args
                            statusCode = HttpStatusCode::kResolveError;
                            response.body().assign(ec.message());

                            if (callbackMethod != nullptr)
                            {
                                callbackMethod->Invoke(r.get(),
                                                       boost::dynamic_pointer_cast<HttpController>(shared_from_this()));
                            }
                        }
                    }
                }
                void OnHandshake(const boost::beast::error_code& ec)
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
                        Ref<NativeScriptImpl> r = script.lock();

                        if (r != nullptr)
                        {
                            // Set event args
                            statusCode = HttpStatusCode::kHandshakeError;
                            response.body().assign(ec.message());

                            if (callbackMethod != nullptr)
                            {
                                callbackMethod->Invoke(r.get(),
                                                       boost::dynamic_pointer_cast<HttpController>(shared_from_this()));
                            }
                        }
                    }
                }
                void OnWrite(const boost::beast::error_code& ec, size_t bytes_transferred)
                {
                    (void)bytes_transferred;

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
                        Ref<NativeScriptImpl> r = script.lock();

                        if (r != nullptr)
                        {
                            // Set event args
                            statusCode = HttpStatusCode::kInternalError;
                            response.body().assign(ec.message());

                            if (callbackMethod != nullptr)
                            {
                                callbackMethod->Invoke(r.get(),
                                                       boost::dynamic_pointer_cast<HttpController>(shared_from_this()));
                            }
                        }
                    }
                }
                void OnRead(const boost::beast::error_code& ec, size_t bytes_transferred)
                {
                    (void)bytes_transferred;

                    if (!ec)
                    {
                        // Invoke event
                        Ref<NativeScriptImpl> r = script.lock();

                        if (r != nullptr)
                        {
                            // Set event args
                            statusCode = (HttpStatusCode)response.result_int();

                            // Invoke callback
                            if (callbackMethod != nullptr)
                            {
                                callbackMethod->Invoke(r.get(),
                                                       boost::dynamic_pointer_cast<HttpController>(shared_from_this()));
                            }
                        }

                        // Gracefully close the socket
                        boost::beast::error_code ec2;
                        stream.next_layer().socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec2);

                        // not_connected happens sometimes so don't bother reporting it
                        if (ec2 && ec2 != boost::beast::errc::not_connected)
                        {
                            // TODO: Error
                            LOG_CODE_MISSING("HTTP Shutdown error");
                        }

                        // Close connection gracefully
                    }
                    else
                    {
                        // Invoke event
                        Ref<NativeScriptImpl> r = script.lock();

                        if (r != nullptr)
                        {
                            // Set event args
                            statusCode = HttpStatusCode::kInternalError;
                            response.body().assign(ec.message());

                            if (callbackMethod != nullptr)
                            {
                                callbackMethod->Invoke(r.get(),
                                                       boost::dynamic_pointer_cast<HttpController>(shared_from_this()));
                            }
                        }
                    }
                }

              public:
                HttpsControllerImpl(const Ref<NativeScriptImpl>& script,
                                    UniqueRef<HttpController::CallbackMethod> callback)
                    : HttpController(script, std::move(callback)), resolver(Worker::GetInstance()->GetContext()),
                      stream(Worker::GetInstance()->GetContext(), sslContext)
                {
                }

                void Send(const std::string& host, uint16_t port, HttpMethod method, const std::string& target,
                          const std::string_view& content, const std::string_view& contentType)
                {
                    // Fill header
                    request.version(11); // HTTP/1.1
                    request.method(methods[(uint8_t)method]);
                    request.target(target);

                    // Set header arguments
                    request.set(boost::beast::http::field::host, host);
                    request.set(boost::beast::http::field::user_agent, "Home. Http Client");
                    request.set(boost::beast::http::field::content_type, contentType.data());

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

            bool Https::Send(const Ref<NativeScriptImpl>& script, const std::string& host, uint16_t port,
                             HttpMethod method, const std::string& target, const std::string_view& content,
                             const std::string_view& contentType,
                             UniqueRef<HttpController::CallbackMethod> methodCallback)
            {
                // Create new controller
                Ref<HttpsControllerImpl> controller =
                    boost::make_shared<HttpsControllerImpl>(script, std::move(methodCallback));

                if (controller != nullptr)
                {
                    // Send http request
                    controller->Send(host, port, method, target, content, contentType);

                    return true;
                }

                return false;
            }
        }
    }
}