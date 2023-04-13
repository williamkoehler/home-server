#pragma once
#include "common.hpp"
#include <home-common/ApiMessage.hpp>
#include <home-users/User.hpp>

namespace server
{
    namespace networking
    {
        class ApiSessionImpl : public ApiSession, public boost::enable_shared_from_this<ApiSessionImpl>
        {
          private:
            boost::asio::strand<websocket_t::executor_type> strand;

            Ref<users::User> user;

            Ref<websocket_t> socket;
            boost::beast::flat_buffer buffer;

            boost::container::vector<Ref<rapidjson::StringBuffer>> messageQueue;

            void OnAccept(const boost::system::error_code& ec);

            void OnRead(const boost::system::error_code& ec, size_t receivedBytes);
            
            void Send(size_t id, const ApiMessage& message);

            void OnWrite(const boost::system::error_code& ec, size_t sentBytes,
                         const Ref<rapidjson::StringBuffer>& message);

            void DoWSShutdown(boost::beast::websocket::close_code code = boost::beast::websocket::close_code::normal,
                              const char* reason = "");
            void DoSSLShutdown(const boost::system::error_code& ec);
            void OnShutdown(const boost::system::error_code& ec);

          public:
            ApiSessionImpl(const Ref<tcp_socket_t>& socket, const Ref<users::User>& user);
            virtual ~ApiSessionImpl();

            void Run(boost::beast::http::request<boost::beast::http::string_body>& request);

            virtual void Send(const ApiMessage& message) override;
            virtual void Send(const Ref<rapidjson::StringBuffer>& message) override;
        };
    }
}