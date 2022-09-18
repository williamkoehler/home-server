#pragma once
#include "common.hpp"
#include <home-users/User.hpp>

namespace server
{
    namespace networking
    {
        class WSSession : public boost::enable_shared_from_this<WSSession>
        {
          private:
            boost::asio::strand<websocket_t::executor_type> strand;

            Ref<users::User> user;

            Ref<websocket_t> socket;
            boost::beast::flat_buffer buffer;

            boost::container::vector<Ref<rapidjson::StringBuffer>> messageQueue;

            void OnAccept(boost::system::error_code error);

            void OnRead(boost::system::error_code error, size_t receivedBytes);

            bool ProcessJsonApi(size_t id, const std::string& msg, rapidjson::Document& input,
                                rapidjson::Document& output);

            void OnWrite(boost::system::error_code error, size_t sentBytes, Ref<rapidjson::StringBuffer> message);

            void DoWSShutdown(boost::beast::websocket::close_code code = boost::beast::websocket::close_code::normal,
                              const char* reason = "");
            void DoSSLShutdown(boost::system::error_code error);
            void OnShutdown(boost::system::error_code error);

          public:
            WSSession(Ref<tcp_socket_t> socket, Ref<users::User> user);
            virtual ~WSSession();

            void Run(boost::beast::http::request<boost::beast::http::string_body>& request);

            void Send(rapidjson::Document& document);
            void Send(rapidjson::StringBuffer& buffer);
            void Send(Ref<rapidjson::StringBuffer> buffer);
        };
    }
}