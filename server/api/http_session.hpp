#pragma once
#include "common.hpp"
#include "user.hpp"

namespace server
{
    namespace api
    {
        class HttpSession : public boost::enable_shared_from_this<HttpSession>
        {
          private:
            boost::asio::strand<tcp_socket_t::executor_type> strand;

            Ref<tcp_socket_t> socket;
            boost::beast::flat_buffer buffer;
            boost::beast::http::request<boost::beast::http::string_body> request;
            rapidjson::StringBuffer responseBuffer;

            void OnRead(const boost::system::error_code& ec, size_t size);

            Ref<api::User> Authenticate();

            void WriteError(const char* error);

            void OnWrite(
                const boost::system::error_code& ec, size_t size,
                const boost::shared_ptr<boost::beast::http::response<boost::beast::http::empty_body>>& response);
            void OnWriteString(
                const boost::system::error_code& ec, size_t size,
                const boost::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>>& response);
            void OnWriteBuffer(
                const boost::system::error_code& ec, size_t size,
                const boost::shared_ptr<boost::beast::http::response<boost::beast::http::buffer_body>>& response);

            void OnShutdown(const boost::system::error_code& ec);

          public:
            HttpSession(const Ref<tcp_socket_t>& socket);
            virtual ~HttpSession();

            void Run();
        };
    }
}