#include "BeaconListener.hpp"
#include "../Core.hpp"
#include <cppcodec/base64_rfc4648.hpp>

namespace server
{
	boost::weak_ptr<BeaconListener> instanceBeacon;

	BeaconListener::BeaconListener()
		: strand(Core::GetInstance()->GetService()->get_executor()),
		nameCopy(Core::GetInstance()->GetName()),
		buffer(),
		listener(*Core::GetInstance()->GetService(), boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 20025))
	{
		externalUrlCopy = Core::GetInstance()->GetExternalAddress() + ":" + std::to_string(Core::GetInstance()->GetExternalPort());
	}
	BeaconListener::~BeaconListener()
	{
	}

	Ref<BeaconListener> BeaconListener::Create(uint16_t port)
	{
		if (!instanceBeacon.expired())
			return Ref<BeaconListener>(instanceBeacon);

		Ref<BeaconListener> beacon = boost::make_shared<BeaconListener>();
		instanceBeacon = beacon;

		beacon->listener.set_option(udp_socket_t::broadcast(true));

		beacon->StartReceiving();

		return beacon;
	}

	void BeaconListener::StartReceiving()
	{
		listener.async_receive_from(boost::asio::buffer(buffer), remoteEnpoint,
			boost::asio::bind_executor(strand, boost::bind(&BeaconListener::OnReceive, this, boost::placeholders::_1, boost::placeholders::_2)));
	}

	void BeaconListener::OnReceive(boost::system::error_code ec, size_t size)
	{
		if (!ec || ec == boost::asio::error::message_size)
		{
			uint8_t digest[SHA256_DIGEST_LENGTH] = "";
			SHA256((const unsigned char*)buffer.data(), strlen(buffer.data()), digest);

			rapidjson::Document document = rapidjson::Document(rapidjson::kObjectType);

			rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

			std::string key = cppcodec::base64_rfc4648::encode((char*)digest, SHA256_DIGEST_LENGTH);
			document.AddMember("key", rapidjson::Value(key.data(), key.size(), allocator), allocator);

			document.AddMember("name", rapidjson::Value(nameCopy.data(), nameCopy.size(), allocator), allocator);

			document.AddMember("ext", rapidjson::Value(externalUrlCopy.data(), externalUrlCopy.size(), allocator), allocator);

			Ref<rapidjson::StringBuffer> message = boost::make_shared<rapidjson::StringBuffer>();
			rapidjson::Writer<rapidjson::StringBuffer> writer = rapidjson::Writer<rapidjson::StringBuffer>(*message);
			document.Accept(writer);

			listener.async_send_to(boost::asio::buffer(message->GetString(), message->GetSize()), remoteEnpoint,
				boost::asio::bind_executor(strand, boost::bind(&BeaconListener::OnSend, this, boost::placeholders::_1, boost::placeholders::_2, message)));
		}

		StartReceiving();
	}
	void BeaconListener::OnSend(boost::system::error_code ec, size_t size, Ref<rapidjson::StringBuffer> message)
	{
	}
}