#pragma once
#include "common.hpp"
#include <home-users/User.hpp>

namespace server
{
    namespace networking
    {
        class HTTPSession : public boost::enable_shared_from_this<HTTPSession>
        {
          private:
            boost::asio::strand<tcp_socket_t::executor_type> strand;

            Ref<tcp_socket_t> socket;
            boost::beast::flat_buffer buffer;
            boost::beast::http::request<boost::beast::http::string_body> request;
            rapidjson::StringBuffer responseBuffer;

            void OnRead(boost::system::error_code error, size_t size);
            
            Ref<users::User> Authenticate();
            
            void WriteError(const char* error);

            void OnWrite(boost::system::error_code error, size_t size,
                         boost::shared_ptr<boost::beast::http::response<boost::beast::http::empty_body>> response);
            void OnWriteString(
                boost::system::error_code error, size_t size,
                boost::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>> response);
            void OnWriteBuffer(
                boost::system::error_code error, size_t size,
                boost::shared_ptr<boost::beast::http::response<boost::beast::http::buffer_body>> response);

            void OnShutdown(boost::system::error_code error);

          public:
            HTTPSession(Ref<tcp_socket_t> socket);
            virtual ~HTTPSession();

            void Run();
        };
    }
}