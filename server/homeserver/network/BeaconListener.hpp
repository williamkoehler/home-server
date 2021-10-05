#pragma once
#include "../common.hpp"
#include <boost/array.hpp>

namespace server
{
	class BeaconListener : public boost::enable_shared_from_this<BeaconListener>
	{
	private:
		boost::asio::strand<ssl_socket_t::executor_type> strand;

		std::string nameCopy = "";
		udp_socket_t listener;
		boost::array<char, 50> buffer;
		boost::asio::ip::udp::endpoint remoteEnpoint;
		
		void StartReceiving();
		void OnReceive(boost::system::error_code ec, size_t size);
		void OnSend(boost::system::error_code ec, size_t size, Ref<rapidjson::StringBuffer> message);

	public:
		BeaconListener();
		~BeaconListener();
		static Ref<BeaconListener> Create(uint16_t port);
	};
}