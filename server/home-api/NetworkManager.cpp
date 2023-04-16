#include "NetworkManager.hpp"
#include "BeaconListener.hpp"
#include "HttpSession.hpp"
#include "WebSocketSession.hpp"
#include "io/DynamicResources.hpp"

namespace server
{
    namespace api
    {
        WeakRef<NetworkManager> instanceNetworkManager;

        NetworkManager::NetworkManager()
        {
        }
        NetworkManager::~NetworkManager()
        {
        }
        Ref<NetworkManager> NetworkManager::Create(const std::string& address, uint16_t port,
                                                   const std::string& externalURL)
        {
            if (!instanceNetworkManager.expired())
                return Ref<NetworkManager>(instanceNetworkManager);

            Ref<NetworkManager> networkManager = boost::make_shared<NetworkManager>();
            instanceNetworkManager = networkManager;

            // Initialize dynamic resources
            networkManager->dynamicResources = DynamicResources::Create();
            if (networkManager->dynamicResources == nullptr)
                return nullptr;

            boost::system::error_code ec;

            // // Initialize ssl/tls context
            // networkManager->context =
            //     boost::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tls_server);
            // networkManager->context->use_certificate_file("ssl-cert.pem",
            // boost::asio::ssl::context::file_format::pem,
            //                                               ec);
            // if (ec)
            // {
            //     LOG_ERROR("Load certificate from 'ssl-cert.pem' -> '{0}'", ec.message());
            //     return nullptr;
            // }

            // networkManager->context->use_private_key_file("ssl-key.pem", boost::asio::ssl::context::file_format::pem,
            //                                               ec);
            // if (ec)
            // {
            //     LOG_ERROR("Load private key from 'ssl-key.pem' -> '{0}'", ec.message());
            //     return nullptr;
            // }

            // Get worker
            Ref<Worker> worker = Worker::GetInstance();
            assert(worker != nullptr);

            // Initialize tcp socket
            networkManager->server = boost::make_shared<boost::asio::ip::tcp::acceptor>(worker->GetContext());

            boost::asio::ip::address addr = boost::asio::ip::make_address(address, ec);
            if (ec)
            {
                LOG_ERROR("Invalid address '{0}' -> '{1}'", address, ec.message());
                return nullptr;
            }

            boost::asio::ip::tcp::endpoint endpoint = boost::asio::ip::tcp::endpoint(addr, port);

            networkManager->server->open(endpoint.protocol(), ec);
            if (ec)
            {
                LOG_ERROR("Open web server -> '{0}'", ec.message());
                return nullptr;
            }

            networkManager->server->set_option(boost::asio::ip::tcp::socket::reuse_address(true), ec);
            if (ec)
            {
                LOG_ERROR("Set web server options -> '{0}'", ec.message());
                return nullptr;
            }

            // Bind tcp socket
            networkManager->server->bind(endpoint, ec);
            if (ec)
            {
                LOG_ERROR("Bind web server:\n{0}", ec.message());
                return nullptr;
            }

            // Let tcp socket listen
            networkManager->server->listen(boost::asio::ip::tcp::socket::max_listen_connections, ec);
            if (ec)
            {
                LOG_ERROR("Make web server listen on port {0}:\n{1}", port, ec.message());
                return nullptr;
            }

            LOG_INFO("Web server listenning on port {0}", port);

            networkManager->socket = boost::make_shared<tcp_socket_t>(worker->GetContext());
            networkManager->server->async_accept(
                networkManager->socket->socket(),
                boost::bind(&NetworkManager::OnAccept, networkManager.get(), boost::placeholders::_1));

            // Initialize beacon listener
            networkManager->beaconListener = BeaconListener::Create(externalURL);
            if (networkManager->beaconListener == nullptr)
            {
                LOG_ERROR("Open beacon listener");
                return nullptr;
            }

            return networkManager;
        }
        Ref<NetworkManager> NetworkManager::GetInstance()
        {
            return Ref<NetworkManager>(instanceNetworkManager);
        }

        void NetworkManager::OnAccept(const boost::system::error_code& ec)
        {
            if (ec)
                return;

            try
            {
                if (!ec)
                {
                    Ref<HttpSession> httpSession = boost::make_shared<HttpSession>(socket);
                    if (httpSession != nullptr)
                    {
                        // Run http session
                        httpSession->Run();
                    }
                    else
                    {
                        LOG_ERROR("Create http session.");
                    }

                    // Get worker
                    Ref<Worker> worker = Worker::GetInstance();
                    assert(worker != nullptr);

                    // Wait for new connection
                    socket = boost::make_shared<tcp_socket_t>(worker->GetContext());
                    server->async_accept(socket->socket(),
                                         boost::bind(&NetworkManager::OnAccept, this, boost::placeholders::_1));
                }
            }
            catch (std::exception e)
            {
                LOG_ERROR("Ops... Internal error...");
            }

            // try
            // {
            //     if (!err)
            //     {
            //         // Start SSL Handshake
            //         socket->async_handshake(
            //             boost::asio::ssl::stream_base::handshake_type::server,
            //             boost::bind(&NetworkManager::OnHandshake, this, boost::placeholders::_1, socket));

            //         // Wait for new connection
            //         socket = boost::make_shared<ssl_socket_t>(worker->GetContext(), GetSSLContext());
            //     }
            // }
            // catch (std::exception e)
            // {
            //     LOG_ERROR("Ops... Internal error:\n{0}", e.what());
            // }

            // server->async_accept(socket->next_layer().socket(),
            //                      boost::bind(&NetworkManager::OnAccept, this, boost::placeholders::_1));
        }

        void NetworkManager::OnHandshake(const boost::system::error_code& ec, const Ref<ssl_socket_t>& socket)
        {
            (void)socket;

            if (ec)
                return;

            // Ref<HTTPSession> httpSession = boost::make_shared<HTTPSession>(socket);
            // if (httpSession != nullptr)
            // {
            //     // Run http session
            //     httpSession->Run();
            // }
            // else
            // {
            //     LOG_ERROR("Create http session.");
            // }
        }
    }
}