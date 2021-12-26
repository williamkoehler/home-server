#include "NetworkManager.hpp"
#include "io/WebPageFiles.hpp"
#include "io/DynamicResources.hpp"
#include "BeaconListener.hpp"
#include "HTTPSession.hpp"
#include "WSSession.hpp"

namespace server
{
	boost::weak_ptr<NetworkManager> instanceNetworkManager;

	NetworkManager::NetworkManager(Ref<boost::asio::io_service> service)
		: service(service)
	{
	}
	NetworkManager::~NetworkManager()
	{
		if (server != nullptr)
		{
			boost::system::error_code ec;
			server->close(ec);
		}
	}
	Ref<NetworkManager> NetworkManager::Create(Ref<boost::asio::io_service> service, std::string address, uint16_t port)
	{
		if (!instanceNetworkManager.expired())
			return Ref<NetworkManager>(instanceNetworkManager);

		Ref<NetworkManager> networkManager = boost::make_shared<NetworkManager>(service);
		instanceNetworkManager = networkManager;

		WebPageFiles::LoadWebPage("www");

		// Initialize dynamic resources
		networkManager->dynamicResources = DynamicResources::Create();
		if (networkManager->dynamicResources == nullptr)
			return nullptr;

		boost::system::error_code ec;

		// Initialize ssl/tls context
		networkManager->context = boost::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tls_server);
		networkManager->context->use_certificate_file("ssl-cert.pem", boost::asio::ssl::context::file_format::pem, ec);
		if (ec)
		{
			LOG_ERROR("Load certificate from 'ssl-cert.pem' -> '{0}'", ec.message());
			return nullptr;
		}

		networkManager->context->use_private_key_file("ssl-key.pem", boost::asio::ssl::context::file_format::pem, ec);
		if (ec)
		{
			LOG_ERROR("Load private key from 'ssl-key.pem' -> '{0}'", ec.message());
			return nullptr;
		}

		// Initialize tcp socket
		networkManager->server = boost::make_shared<boost::asio::ip::tcp::acceptor>(*networkManager->service);

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

		// Bind tcp socker
		networkManager->server->bind(endpoint, ec);
		if (ec)
		{
			LOG_ERROR("Bind web server -> '{0}'", ec.message());
			return nullptr;
		}

		// Let tcp socket listen
		networkManager->server->listen(boost::asio::ip::tcp::socket::max_listen_connections, ec);
		if (ec)
		{
			LOG_ERROR("Make web server listen on port {0} -> '{1}'", port, ec.message());
			return nullptr;
		}

		LOG_INFO("Web server listenning on port {0}", port);

		networkManager->socket = boost::make_shared<ssl_socket_t>(*(networkManager->service), *(networkManager->context));
		networkManager->server->async_accept(networkManager->socket->next_layer().socket(), boost::bind(&NetworkManager::OnAccept, networkManager.get(), boost::placeholders::_1));

		// Initialize beacon listener
		networkManager->beaconListener = BeaconListener::Create(port);
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
		boost::lock_guard lock(mutex);

		if (!err)
		{
			// Start SSL Handshake
			socket->async_handshake(boost::asio::ssl::stream_base::handshake_type::server, boost::bind(&NetworkManager::OnHandshake, this, boost::placeholders::_1, socket));

			// Wait for new connection
			socket = boost::make_shared<ssl_socket_t>(*service, *context);
		}

		server->async_accept(socket->next_layer().socket(), boost::bind(&NetworkManager::OnAccept, this, boost::placeholders::_1));
	}

	void NetworkManager::OnHandshake(boost::system::error_code error, Ref<ssl_socket_t> socket)
	{
		if (error)
			return;

		Ref<HTTPSession> httpSession = boost::make_shared<HTTPSession>(socket);
		httpSession->Run();
	}

	void NetworkManager::Broadcast(rapidjson::Document &document)
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