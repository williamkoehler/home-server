#include "NetworkManager.hpp"
#include "BeaconListener.hpp"
#include "HTTPSession.hpp"
#include "WSSession.hpp"
#include "io/DynamicResources.hpp"

namespace server
{
    namespace networking
    {
        WeakRef<NetworkManager> instanceNetworkManager;

        NetworkManager::NetworkManager(Ref<threading::Worker> worker) : worker(std::move(worker))
        {
        }
        NetworkManager::~NetworkManager()
        {
        }
        Ref<NetworkManager> NetworkManager::Create(Ref<threading::Worker> worker, const std::string& address,
                                                   uint16_t port, const std::string& externalURL)
        {
            assert(worker != nullptr);

            if (!instanceNetworkManager.expired())
                return Ref<NetworkManager>(instanceNetworkManager);

            Ref<NetworkManager> networkManager = boost::make_shared<NetworkManager>(std::move(worker));
            instanceNetworkManager = networkManager;

            // Initialize dynamic resources
            networkManager->dynamicResources = DynamicResources::Create();
            if (networkManager->dynamicResources == nullptr)
                return nullptr;

            boost::system::error_code ec;

            // Initialize ssl/tls context
            networkManager->context =
                boost::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tls_server);
            networkManager->context->use_certificate_file("ssl-cert.pem", boost::asio::ssl::context::file_format::pem,
                                                          ec);
            if (ec)
            {
                LOG_ERROR("Load certificate from 'ssl-cert.pem' -> '{0}'", ec.message());
                return nullptr;
            }

            networkManager->context->use_private_key_file("ssl-key.pem", boost::asio::ssl::context::file_format::pem,
                                                          ec);
            if (ec)
            {
                LOG_ERROR("Load private key from 'ssl-key.pem' -> '{0}'", ec.message());
                return nullptr;
            }

            // Initialize tcp socket
            networkManager->server =
                boost::make_shared<boost::asio::ip::tcp::acceptor>(networkManager->worker->GetContext());

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

            networkManager->socket =
                boost::make_shared<ssl_socket_t>(networkManager->worker->GetContext(), networkManager->GetSSLContext());
            networkManager->server->async_accept(
                networkManager->socket->next_layer().socket(),
                boost::bind(&NetworkManager::OnAccept, networkManager.get(), boost::placeholders::_1));

            // Initialize beacon listener
            networkManager->beaconListener = BeaconListener::Create(networkManager->worker, externalURL);
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

        void NetworkManager::OnAccept(boost::system::error_code err)
        {
            // Lock main mutex
            boost::lock_guard lock(mutex);

            try
            {
                if (!err)
                {
                    // Start SSL Handshake
                    socket->async_handshake(
                        boost::asio::ssl::stream_base::handshake_type::server,
                        boost::bind(&NetworkManager::OnHandshake, this, boost::placeholders::_1, socket));

                    // Wait for new connection
                    socket = boost::make_shared<ssl_socket_t>(worker->GetContext(), GetSSLContext());
                }
            }
            catch (std::exception e)
            {
                LOG_ERROR("Ops... Internal error:\n{0}", e.what());
            }

            server->async_accept(socket->next_layer().socket(),
                                 boost::bind(&NetworkManager::OnAccept, this, boost::placeholders::_1));
        }

        void NetworkManager::OnHandshake(boost::system::error_code error, Ref<ssl_socket_t> socket)
        {
            if (error)
                return;

            Ref<HTTPSession> httpSession = boost::make_shared<HTTPSession>(socket);
            if (httpSession != nullptr)
            {
                // Run http session
                httpSession->Run();
            }
            else
            {
                LOG_ERROR("Create http session.");
            }
        }

        void NetworkManager::Broadcast(rapidjson::Document& document)
        {
            boost::lock_guard lock(mutex);

            Ref<rapidjson::StringBuffer> message = boost::make_shared<rapidjson::StringBuffer>();
            rapidjson::Writer<rapidjson::StringBuffer> writer = rapidjson::Writer<rapidjson::StringBuffer>(*message);
            document.Accept(writer);

            for (size_t i = 0; i < sessionList.size(); i++)
            {
                if (Ref<WSSession> session = sessionList[i].lock())
                    session->Send(message);
                else
                {
                    sessionList.erase(sessionList.begin() + i);
                    i--; // Beacuse a session has been removed the iterator needs to decrease
                }
            }
        }
    }
}