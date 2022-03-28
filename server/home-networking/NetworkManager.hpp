#pragma once
#include "common.hpp"
#include <home-threading/Worker.hpp>

namespace server
{
    namespace networking
    {
        class BeaconListener;
        class DynamicResources;
        class WSSession;

        class NetworkManager : public std::enable_shared_from_this<NetworkManager>
        {
          private:
            friend class WSSession;

            boost::mutex mutex;

            const Ref<threading::Worker> worker;

            Ref<DynamicResources> dynamicResources = nullptr;

            Ref<boost::asio::ssl::context> context = nullptr;
            Ref<boost::asio::ip::tcp::acceptor> server = nullptr;
            Ref<ssl_socket_t> socket = nullptr;

            Ref<BeaconListener> beaconListener = nullptr;

            void OnAccept(boost::system::error_code err);
            void OnHandshake(boost::system::error_code err, Ref<ssl_socket_t> socket);

            boost::container::vector<WeakRef<WSSession>> sessionList;

          public:
            NetworkManager(Ref<threading::Worker> worker);
            virtual ~NetworkManager();
            static Ref<NetworkManager> Create(Ref<threading::Worker> worker, const std::string& address, uint16_t port,
                                              const std::string& externalURL);
            static Ref<NetworkManager> GetInstance();

            /// @brief Get networking worker
            /// 
            /// @return Worker
            inline Ref<threading::Worker> GetWorker() const
            {
                return worker;
            }

            inline boost::asio::ssl::context& GetSSLContext() const
            {
                return *context;
            }

            void Broadcast(rapidjson::Document& document);
        };
    }
}