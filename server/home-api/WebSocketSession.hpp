#pragma once
#include "Message.hpp"
#include "User.hpp"
#include "common.hpp"

namespace server
{
    namespace api
    {
        class WebSocketSession;

        using WebSocketApiCallDefinition = void (*)(const Ref<api::User>&, const ApiRequestMessage&,
                                                    ApiResponseMessage&, const Ref<WebSocketSession>&);

        class WebSocketSession final : public boost::enable_shared_from_this<WebSocketSession>
        {
          private:
            boost::asio::strand<websocket_t::executor_type> strand;

            Ref<api::User> user;

            Ref<websocket_t> socket;
            boost::beast::flat_buffer buffer;

            boost::container::vector<Ref<rapidjson::StringBuffer>> messageQueue;

            void OnAccept(const boost::system::error_code& ec);

            void OnRead(const boost::system::error_code& ec, size_t receivedBytes);

            void Send(size_t id, const ApiResponseMessage& message);

            void OnWrite(const boost::system::error_code& ec, size_t sentBytes,
                         const Ref<rapidjson::StringBuffer>& message);

            void DoWSShutdown(boost::beast::websocket::close_code code = boost::beast::websocket::close_code::normal,
                              const char* reason = "");
            void DoSSLShutdown(const boost::system::error_code& ec);
            void OnShutdown(const boost::system::error_code& ec);

          public:
            WebSocketSession(const Ref<tcp_socket_t>& socket, const Ref<api::User>& user);
            virtual ~WebSocketSession();

            /// @brief Get websocket api map
            ///
            /// @return robin_hood::unordered_node_map<std::string, WebSocketApiCallDefinition> Api map
            static robin_hood::unordered_node_map<std::string, WebSocketApiCallDefinition>& GetApiMap();

            void Run(boost::beast::http::request<boost::beast::http::string_body>& request);

            void Send(const Ref<rapidjson::StringBuffer>& message);
        };
    }
}