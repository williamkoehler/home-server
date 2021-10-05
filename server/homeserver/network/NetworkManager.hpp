#pragma once
#include "../common.hpp"

namespace server
{
	class BeaconListener;
	class WSSession;

	class DeviceManager;
	class Device;

	class BroadcastContainer;

	class NetworkManager : public std::enable_shared_from_this<NetworkManager>
	{
	private:
		friend class WSSession;

		boost::mutex mutex;

		Ref<boost::asio::io_service> service;

		Ref<boost::asio::ssl::context> context = nullptr;
		Ref<boost::asio::ip::tcp::acceptor> server = nullptr;
		Ref<ssl_socket_t> socket = nullptr;
		Ref<BeaconListener> beaconListener = nullptr;

		void OnAccept(boost::system::error_code error);
		void OnHandshake(boost::system::error_code error, Ref<ssl_socket_t> socket);

		boost::container::vector<WeakRef<WSSession>> sessionList;

	public:
		NetworkManager(Ref<boost::asio::io_service> service);
		~NetworkManager();
		static Ref<NetworkManager> Create(Ref<boost::asio::io_service> service, std::string address, uint16_t port);
		static Ref<NetworkManager> GetInstance();

		void Broadcast(rapidjson::Document& document);
	};
}