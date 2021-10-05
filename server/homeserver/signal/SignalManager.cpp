#include "SignalManager.hpp"
#include "../Core.hpp"
#include "../home/Device.hpp"
#include "../home/DeviceManager.hpp"
#include "../json/JsonApi.hpp"
#include "../network/NetworkManager.hpp"

namespace server
{
	boost::weak_ptr<SignalManager> instanceSignals;

	SignalManager::SignalManager()
		: startTime(boost::posix_time::microsec_clock::universal_time()), signalCount(0)
	{
	}
	SignalManager::~SignalManager()
	{
	}

	Ref<SignalManager> SignalManager::Create()
	{
		if (!instanceSignals.expired())
			return Ref<SignalManager>(instanceSignals);

		Ref<SignalManager> signalManager = boost::make_shared<SignalManager>();
		instanceSignals = signalManager;

		return signalManager;
	}

	Ref<SignalManager> SignalManager::GetInstance()
	{
		return Ref<SignalManager>(instanceSignals);
	}

	void SignalManager::ResetCounter()
	{
		startTime = boost::posix_time::microsec_clock::universal_time();
		signalCount = 0;
	}
	void SignalManager::Update()
	{
		const boost::posix_time::ptime endTime = boost::posix_time::microsec_clock::universal_time();
		boost::posix_time::time_duration deltaTime = endTime - startTime;

		/*int64_t cycleDuration = deltaTime.total_milliseconds();
		if (cycleDuration > 1000)
			LOG_WARNING("Detected long update cycles of {0}ms > 1000ms", cycleDuration);*/

		//float avg = (float)cycleDuration / signalCount;
	}

	void SignalManager::OnHomeChanged()
	{
		signalCount++;

		rapidjson::Document document = rapidjson::Document(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		document.AddMember("msg", rapidjson::Value("set-home?info", 13, allocator), allocator);

		JsonApi::BuildJsonHome(document, allocator);

		NetworkManager::GetInstance()->Broadcast(document);
	}
	void SignalManager::OnRoomChanged(Ref<home::Room> room)
	{
		signalCount++;

		rapidjson::Document document = rapidjson::Document(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		document.AddMember("msg", rapidjson::Value("set-room?info", 13, allocator), allocator);

		JsonApi::BuildJsonRoom(boost::static_pointer_cast<Room>(room), document, allocator);

		NetworkManager::GetInstance()->Broadcast(document);
	}
	
	void SignalManager::OnDeviceChanged(Ref<home::Device> device)
	{
		signalCount++;

		rapidjson::Document document = rapidjson::Document(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		document.AddMember("msg", rapidjson::Value("set-device?info", 15, allocator), allocator);

		JsonApi::BuildJsonDevice(boost::static_pointer_cast<Device>(device), document, allocator);

		NetworkManager::GetInstance()->Broadcast(document);
	}
	void SignalManager::OnDevicePropertiesChanged(Ref<home::Device> device)
	{
		signalCount++;

		rapidjson::Document document = rapidjson::Document(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		document.AddMember("msg", rapidjson::Value("set-device?state", 16, allocator), allocator);

		JsonApi::BuildJsonDeviceState(boost::static_pointer_cast<Device>(device), document, allocator);

		NetworkManager::GetInstance()->Broadcast(document);
	}
	
	void SignalManager::OnDeviceManagerChanged(Ref<home::DeviceManager> deviceManager)
	{
		signalCount++;

		rapidjson::Document document = rapidjson::Document(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		document.AddMember("msg", rapidjson::Value("set-devicemanager?info", 22, allocator), allocator);

		JsonApi::BuildJsonDeviceManager(boost::static_pointer_cast<DeviceManager>(deviceManager), document, allocator);

		NetworkManager::GetInstance()->Broadcast(document);
	}
	void SignalManager::OnDeviceManagerPropertiesChanged(Ref<home::DeviceManager> deviceManager)
	{
		signalCount++;

		rapidjson::Document document = rapidjson::Document(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		document.AddMember("msg", rapidjson::Value("set-devicemanager?state", 23, allocator), allocator);

		JsonApi::BuildJsonDeviceManagerState(boost::static_pointer_cast<DeviceManager>(deviceManager), document, allocator);

		NetworkManager::GetInstance()->Broadcast(document);
	}
}