#pragma once
#include "common.hpp"
#include <common/worker.hpp>

namespace server
{
    namespace api
    {
        class BeaconListener;
        class DynamicResources;
        class WebSocketSession;

        class NetworkManager : public std::enable_shared_from_this<NetworkManager>
        {
          private:
            friend class WebSocketSession;

            Ref<DynamicResources> dynamicResources = nullptr;

            // Ref<boost::asio::ssl::context> context = nullptr;
            Ref<boost::asio::ip::tcp::acceptor> server = nullptr;
            Ref<tcp_socket_t> socket = nullptr;

            Ref<BeaconListener> beaconListener = nullptr;

            void OnAccept(const boost::system::error_code& ec);
            void OnHandshake(const boost::system::error_code& ec, const Ref<ssl_socket_t>& socket);

          public:
            NetworkManager();
            virtual ~NetworkManager();
            static Ref<NetworkManager> Create(const std::string& address, uint16_t port,
                                              const std::string& externalURL);
            static Ref<NetworkManager> GetInstance();
        };
    }
}