#include "Home.hpp"
#include "../Core.hpp"
#include "Room.hpp"
#include "Device.hpp"
#include "DeviceManager.hpp"
#include "Action.hpp"
#include "../signal/SignalManager.hpp"
#include "../plugin/PluginManager.hpp"
#include <xxHash/xxhash.h>
#include "../network/NetworkManager.hpp"
#include "../json/JsonApi.hpp"

namespace server
{
	boost::weak_ptr<Home> instanceHome;

	Home::Home()
	{
	}
	Home::~Home()
	{
		roomList.clear();
		deviceManagerList.clear();
	}
	Ref<Home> Home::Create()
	{
		if (!instanceHome.expired())
			return Ref<Home>(instanceHome);

		Ref<Home> home = boost::make_shared<Home>();
		if (home == nullptr)
			return nullptr;

		instanceHome = home;

		try
		{
			// Load
			home->Load();
		}
		catch (std::exception)
		{
			return nullptr;
		}

		home->UpdateTimestamp();

		return home;
	}
	Ref<Home> Home::GetInstance()
	{
		return Ref<Home>(instanceHome);
	}

	//! Timestamp
	void Home::UpdateTimestamp()
	{
		const time_t ts = time(nullptr);
		timestamp = ts;
	}

	//! Room
	Ref<Room> Home::AddRoom(std::string name, uint32_t roomID, uint16_t type, rapidjson::Value& json)
	{
		uint32_t genID = roomID ? roomID : XXH32(name.c_str(), name.size(), 0x524F4F4D);

		// Check for duplicate
		{
			boost::shared_lock_guard lock(mutex);

			size_t pass = 10;
			while (roomList.count(genID))
			{
				genID++;

				//Only allow 10 passes
				if (!(pass--))
				{
					LOG_ERROR("Generate unique id for room '{0}'", name);
					return nullptr;
				}
			}
		}

		Ref<Room> room = Room::Create(name, genID, type);
		if (room == nullptr)
			return nullptr;

		room->Load(json);

		boost::lock_guard lock(mutex);
		roomList[genID] = room;

		UpdateTimestamp();

		return room;
	}

	Ref<home::Room> Home::GetRoom(uint32_t roomID)
	{
		boost::shared_lock_guard lock(mutex);

		const boost::unordered::unordered_map<uint32_t, Ref<Room>>::const_iterator it = roomList.find(roomID);
		if (it == roomList.end())
			return nullptr;

		return (*it).second;
	}
	Ref<Room> Home::GetRoom_(uint32_t roomID)
	{
		boost::shared_lock_guard lock(mutex);

		const boost::unordered::unordered_map<uint32_t, Ref<Room>>::const_iterator it = roomList.find(roomID);
		if (it == roomList.end())
			return nullptr;

		return (*it).second;
	}

	void Home::RemoveRoom(uint32_t roomID)
	{
		boost::lock_guard lock(mutex);

		const boost::unordered::unordered_map<uint32_t, Ref<Room>>::const_iterator it = roomList.find(roomID);
		if (it == roomList.end())
			throw std::runtime_error("Room ID does not exist");

		roomList.erase(it);

		UpdateTimestamp();
	}

	Ref<Device> Home::AddDevice(std::string name, uint32_t deviceID, uint32_t scriptID, rapidjson::Value& json)
	{
		uint32_t genID = deviceID ? deviceID : XXH32(name.c_str(), name.size(), 0x44455649);

		// Check for duplicate
		{
			boost::shared_lock_guard lock(mutex);

			size_t pass = 10;
			while (deviceList.count(genID))
			{
				genID++;

				//Only allow 10 passes
				if (!(pass--))
				{
					LOG_ERROR("Failing to generate unique id for device '{0}'", name);
					return nullptr;
				}
			}
		}

		Ref<Device> device = Device::Create(name, genID, scriptID, json);
		if (!device)
			return nullptr;

		boost::lock_guard lock(mutex);
		deviceList[genID] = device;

		UpdateTimestamp();

		return device;
	}

	Ref<home::Device> Home::GetDevice(uint32_t deviceID)
	{
		boost::shared_lock_guard lock(mutex);

		const boost::unordered::unordered_map<uint32_t, Ref<Device>>::const_iterator it = deviceList.find(deviceID);
		if (it == deviceList.end())
			return nullptr;

		return (*it).second;
	}
	Ref<Device> Home::GetDevice_(uint32_t deviceID)
	{
		boost::shared_lock_guard lock(mutex);

		const boost::unordered::unordered_map<uint32_t, Ref<Device>>::const_iterator it = deviceList.find(deviceID);
		if (it == deviceList.end())
			return nullptr;

		return (*it).second;
	}

	void Home::RemoveDevice(uint32_t deviceID)
	{
		boost::lock_guard lock(mutex);

		const boost::unordered::unordered_map<uint32_t, Ref<Device>>::const_iterator it = deviceList.find(deviceID);
		if (it == deviceList.end())
			throw std::runtime_error("Device ID does not exist");

		for (std::pair<uint32_t, Ref<Room>> pair : roomList)
			pair.second->RemoveDevice(it->second);

		deviceList.erase(it);

		UpdateTimestamp();
	}

	void Home::AddDeviceToUpdatables(Ref<Device> device)
	{
		boost::lock_guard lock(mutex);
		deviceUpdateList.push_back(device);
	}

	//! DeviceManager
	Ref<DeviceManager> Home::AddDeviceManager(std::string name, uint32_t managerID, uint32_t scriptID, rapidjson::Value& json)
	{
		uint32_t genID = managerID ? managerID : XXH32(name.c_str(), name.size(), 0x444D414E);

		// Check for duplicate
		{
			boost::shared_lock_guard lock(mutex);

			size_t pass = 10;
			while (deviceManagerList.count(genID))
			{
				genID++;

				//Only allow 10 passes
				if (!(pass--))
				{
					LOG_ERROR("Failing to generate unique id for device manager '{0}'", name);
					return nullptr;
				}
			}
		}

		Ref<DeviceManager> deviceManager = DeviceManager::Create(name, genID, scriptID, json);
		if (!deviceManager)
			return nullptr;

		boost::lock_guard lock(mutex);
		deviceManagerList[genID] = deviceManager;

		UpdateTimestamp();

		return deviceManager;
	}

	Ref<home::DeviceManager> Home::GetDeviceManager(uint32_t managerID)
	{
		boost::shared_lock_guard lock(mutex);

		const boost::unordered::unordered_map<uint32_t, Ref<DeviceManager>>::const_iterator it = deviceManagerList.find(managerID);
		if (it == deviceManagerList.end())
			return nullptr;

		return (*it).second;
	}
	Ref<DeviceManager> Home::GetDeviceManager_(uint32_t managerID)
	{
		boost::shared_lock_guard lock(mutex);

		const boost::unordered::unordered_map<uint32_t, Ref<DeviceManager>>::const_iterator it = deviceManagerList.find(managerID);
		if (it == deviceManagerList.end())
			return nullptr;

		return (*it).second;
	}

	void Home::RemoveDeviceManager(uint32_t managerID)
	{
		boost::lock_guard lock(mutex);

		const boost::unordered::unordered_map<uint32_t, Ref<DeviceManager>>::const_iterator it = deviceManagerList.find(managerID);
		if (it == deviceManagerList.end())
			throw std::runtime_error("DeviceManager ID does not exist");

		deviceManagerList.erase(it);

		UpdateTimestamp();
	}

	void Home::AddDeviceManagerToUpdatables(Ref<DeviceManager> deviceManager)
	{
		boost::lock_guard lock(mutex);
		deviceManagerUpdateList.push_back(deviceManager);
	}

	//! Action
	Ref<Action> Home::AddAction(std::string name, uint32_t actionID, uint32_t sourceID, rapidjson::Value& json)
	{
		uint32_t genID = actionID ? actionID : XXH32(name.c_str(), name.size(), 0x41435449);

		{
			boost::shared_lock_guard lock(mutex);

			size_t pass = 10;
			while (actionList.count(genID))
			{
				genID++;

				//Only allow 10 passes
				if (!(pass--))
				{
					LOG_ERROR("Failing to generate unique id for action '{0}'", name);
					return nullptr;
				}
			}
		}

		Ref<Action> action = Action::Create(name, genID, sourceID);
		if (!action)
			return nullptr;

		boost::lock_guard lock(mutex);
		actionList[genID] = action;

		UpdateTimestamp();

		return action;
	}

	Ref<home::Action> Home::GetAction(uint32_t actionID)
	{
		boost::shared_lock_guard lock(mutex);

		const boost::unordered::unordered_map<uint32_t, Ref<Action>>::const_iterator it = actionList.find(actionID);
		if (it == actionList.end())
			return nullptr;

		return (*it).second;
	}
	Ref<Action> Home::GetAction_(uint32_t actionID)
	{
		boost::shared_lock_guard lock(mutex);

		const boost::unordered::unordered_map<uint32_t, Ref<Action>>::const_iterator it = actionList.find(actionID);
		if (it == actionList.end())
			return nullptr;

		return (*it).second;
	}

	void Home::RemoveAction(uint32_t actionID)
	{
		boost::lock_guard lock(mutex);

		const boost::unordered::unordered_map<uint32_t, Ref<Action>>::const_iterator it = actionList.find(actionID);
		if (it == actionList.end())
			throw std::runtime_error("Action ID does not exist");

		actionList.erase(it);

		UpdateTimestamp();
	}

	void Home::Update()
	{
		// Wait 500ms for older worker threads to finish
		if (workerMutex.try_lock())
		{
			if (cycleCount < SIZE_MAX)
				cycleCount++;
			else
				cycleCount = 0;

			Ref<boost::asio::io_service> service = Core::GetInstance()->GetWorkerService();

			boost::asio::post(service->get_executor(), boost::bind(&Home::Worker, shared_from_this()));

			workerMutex.unlock();
		}
	}

	void Home::Worker()
	{
		boost::lock_guard lock(workerMutex);

		rapidjson::Document document = rapidjson::Document(rapidjson::kObjectType);

		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		Ref<SignalManager> signalManager = SignalManager::GetInstance();

		// Update signal manager and warn user when busy
		signalManager->Update();

		//Update device managers
		for (size_t i = 0; i < deviceManagerUpdateList.size(); i++)
		{
			WeakRef<DeviceManager>& ptr = deviceManagerUpdateList[i];

			if (Ref<DeviceManager> deviceManager = ptr.lock())
				deviceManager->Update(signalManager, cycleCount);
			else
			{
				// Remove item from list
				deviceManagerUpdateList.erase(deviceManagerUpdateList.begin() + i);
				i--;
			}
		}

		rapidjson::Value deviceListJson = rapidjson::Value(rapidjson::kArrayType);

		//Update devices
		for (size_t i = 0; i < deviceUpdateList.size(); i++)
		{
			WeakRef<Device>& ptr = deviceUpdateList[i];

			if (Ref<Device> device = ptr.lock())
				device->Update(signalManager, cycleCount);
			else
			{
				// Remove item from list
				deviceUpdateList.erase(deviceUpdateList.begin() + i);
				i--;
			}
		}
	}

	//! IO
	void Home::Load()
	{
		LOG_INFO("Loading home information from 'home-info.json'");

		FILE* file = fopen("home-info.json", "r");
		if (file == nullptr)
		{
			LOG_ERROR("Open/find 'home-info.json'");
			throw std::runtime_error("Open/find file 'home-info.json'");
		}

		char buffer[RAPIDJSON_BUFFER_SIZE_SMALL];
		rapidjson::FileReadStream stream(file, buffer, sizeof(buffer));

		rapidjson::Document document;
		if (document.ParseStream(stream).HasParseError() || !document.IsObject())
		{
			LOG_ERROR("Read 'home-info.json'");
			throw std::runtime_error("Read file 'home-info.json'");
		}

		// Read device manager list
		{
			rapidjson::Value::MemberIterator deviceManagerListIt = document.FindMember("device-manager-list");
			if (deviceManagerListIt != document.MemberEnd() && deviceManagerListIt->value.IsArray())
			{
				rapidjson::Value& deviceManagerListJson = deviceManagerListIt->value;
				for (rapidjson::Value::ValueIterator deviceManagerIt = deviceManagerListJson.Begin(); deviceManagerIt != deviceManagerListJson.End(); deviceManagerIt++)
				{
					if (!deviceManagerIt->IsObject())
					{
						LOG_ERROR("Missing device manager in 'device-manager-list' in 'home-info.json'");
						throw std::runtime_error("Invalid file 'home-info.json'");
					}

					//Read name
					rapidjson::Value::MemberIterator nameIt = deviceManagerIt->FindMember("name");
					if (nameIt == deviceManagerIt->MemberEnd() || !nameIt->value.IsString())
					{
						LOG_ERROR("Missing 'name' in device manager in 'device-manager-list' in 'home-info.json'");
						throw std::runtime_error("Invalid file 'home-info.json'");
					}

					//Read id
					rapidjson::Value::MemberIterator idIt = deviceManagerIt->FindMember("id");
					if (idIt == deviceManagerIt->MemberEnd() || !idIt->value.IsUint())
					{
						LOG_ERROR("Missing 'id' in device manager in 'device-manager-list' in 'home-info.json'");
						throw std::runtime_error("Invalid file 'home-info.json'");
					}

					//Read type
					rapidjson::Value::MemberIterator scriptIDIt = deviceManagerIt->FindMember("script-id");
					if (scriptIDIt == deviceManagerIt->MemberEnd() || !scriptIDIt->value.IsUint())
					{
						LOG_ERROR("Missing 'script-id' in device manager in 'device-manager-list' in 'home-info.json'");
						throw std::runtime_error("Invalid file 'home-info.json'");
					}

					std::string name = std::string(nameIt->value.GetString(), nameIt->value.GetStringLength());

					Ref<DeviceManager> deviceManager = AddDeviceManager(
						name,
						idIt->value.GetUint(),
						scriptIDIt->value.GetUint(),
						*deviceManagerIt);
					if (deviceManager == nullptr)
					{
						LOG_ERROR("Add device manager '{0}'", name);
						throw std::runtime_error("Invalid file 'home-info.json'");
					}
				}
			}
		}

		//Read device list
		{
			rapidjson::Value::MemberIterator deviceListIt = document.FindMember("device-list");
			if (deviceListIt != document.MemberEnd() && deviceListIt->value.IsArray())
			{
				rapidjson::Value& deviceListJson = deviceListIt->value;
				for (rapidjson::Value::ValueIterator deviceIt = deviceListJson.Begin(); deviceIt != deviceListJson.End(); deviceIt++)
				{
					if (!deviceIt->IsObject())
					{
						LOG_ERROR("Missing device in 'device-list' in 'home-info.json'");
						throw std::runtime_error("Invalid file 'home-info.json'");
					}

					//Read name
					rapidjson::Value::MemberIterator nameIt = deviceIt->FindMember("name");
					if (nameIt == deviceIt->MemberEnd() || !nameIt->value.IsString())
					{
						LOG_ERROR("Missing 'name' in device in 'device-list' in 'home-info.json'");
						throw std::runtime_error("Invalid file 'home-info.json'");
					}

					//Read id
					rapidjson::Value::MemberIterator idIt = deviceIt->FindMember("id");
					if (idIt == deviceIt->MemberEnd() || !idIt->value.IsUint())
					{
						LOG_ERROR("Missing 'id' in device in 'device-list' in 'home-info.json'");
						throw std::runtime_error("Invalid file 'home-info.json'");
					}

					//Read script-id
					rapidjson::Value::MemberIterator scriptIDIt = deviceIt->FindMember("script-id");
					if (scriptIDIt == deviceIt->MemberEnd() || !scriptIDIt->value.IsUint())
					{
						LOG_ERROR("Missing 'script-id' in device in 'device-list' in 'home-info.json'");
						throw std::runtime_error("Invalid file 'home-info.json'");
					}

					std::string name = std::string(nameIt->value.GetString(), nameIt->value.GetStringLength());

					Ref<Device> device = AddDevice(
						name,
						idIt->value.GetUint(),
						scriptIDIt->value.GetUint(),
						*deviceIt);
					if (device == nullptr)
					{
						LOG_ERROR("Add device '{0}'", name);
						throw std::runtime_error("Invalid file 'home-info.json'");
					}
				}
			}
		}

		// Read action list
		{
			rapidjson::Value::MemberIterator actionListIt = document.FindMember("action-list");
			if (actionListIt != document.MemberEnd() && actionListIt->value.IsArray())
			{
				rapidjson::Value& actionListJson = actionListIt->value;
				for (rapidjson::Value::ValueIterator actionIt = actionListJson.Begin(); actionIt != actionListJson.End(); actionIt++)
				{
					if (!actionIt->IsObject())
					{
						LOG_ERROR("Missing action in 'action-list' in 'home-info.json'");
						throw std::runtime_error("Invalid file 'home-info.json'");
					}

					//Read name
					rapidjson::Value::MemberIterator nameIt = actionIt->FindMember("name");
					if (nameIt == actionIt->MemberEnd() || !nameIt->value.IsString())
					{
						LOG_ERROR("Missing 'name' in action in 'action-list' in 'home-info.json'");
						throw std::runtime_error("Invalid file 'home-info.json'");
					}

					//Read id
					rapidjson::Value::MemberIterator idIt = actionIt->FindMember("id");
					if (idIt == actionIt->MemberEnd() || !idIt->value.IsUint())
					{
						LOG_ERROR("Missing 'id' in action in 'action-list' in 'home-info.json'");
						throw std::runtime_error("Invalid file 'home-info.json'");
					}

					//Read source-id
					rapidjson::Value::MemberIterator sourceIDIt = actionIt->FindMember("source-id");
					if (sourceIDIt == actionIt->MemberEnd() || !sourceIDIt->value.IsUint())
					{
						LOG_ERROR("Missing 'source-id' in action in 'action-list' in 'home-info.json'");
						throw std::runtime_error("Invalid file 'home-info.json'");
					}

					std::string name = std::string(nameIt->value.GetString(), nameIt->value.GetStringLength());

					Ref<Action> action = AddAction(
						name,
						idIt->value.GetUint(),
						sourceIDIt->value.GetUint(),
						*actionIt);
					if (action == nullptr)
					{
						LOG_ERROR("Add action '{0}'", name);
						throw std::runtime_error("Invalid file 'home-info.json'");
					}
				}
			}
		}

		// Read room list
		{
			rapidjson::Value::MemberIterator roomListIt = document.FindMember("room-list");
			if (roomListIt != document.MemberEnd() && roomListIt->value.IsArray())
			{
				rapidjson::Value& roomListJson = roomListIt->value;
				for (rapidjson::Value::ValueIterator roomIt = roomListJson.Begin(); roomIt != roomListJson.End(); roomIt++)
				{
					if (!roomIt->IsObject())
					{
						LOG_ERROR("Missing room in 'room-list' in 'home-info.json'");
						throw std::runtime_error("Invalid file 'home-info.json'");
					}

					//Read name
					rapidjson::Value::MemberIterator nameIt = roomIt->FindMember("name");
					if (nameIt == roomIt->MemberEnd() || !nameIt->value.IsString())
					{
						LOG_ERROR("Missing 'name' in room in 'room-list' in 'home-info.json'");
						throw std::runtime_error("Invalid file 'home-info.json'");
					}

					//Read id
					rapidjson::Value::MemberIterator idIt = roomIt->FindMember("id");
					if (idIt == roomIt->MemberEnd() || !idIt->value.IsUint())
					{
						LOG_ERROR("Missing 'id' in room in 'room-list' in 'home-info.json'");
						throw std::runtime_error("Invalid file 'home-info.json'");
					}

					//Read type
					rapidjson::Value::MemberIterator typeIt = roomIt->FindMember("type");
					if (typeIt == roomIt->MemberEnd() || !typeIt->value.IsUint())
					{
						LOG_ERROR("Missing 'type' in room in 'room-list' in 'home-info.json'");
						throw std::runtime_error("Invalid file 'home-info.json'");
					}

					std::string name = std::string(nameIt->value.GetString(), nameIt->value.GetStringLength());

					Ref<Room> room = AddRoom(
						name,
						idIt->value.GetUint(),
						typeIt->value.GetUint(),
						*roomIt);
					if (room == nullptr)
					{
						LOG_ERROR("Add room '{0}'", name);
						throw std::runtime_error("Invalid file 'home-info.json'");
					}
				}
			}
		}

		fclose(file);
	}
	void Home::Save()
	{
		boost::shared_lock_guard lock(mutex);

		LOG_INFO("Saving home information to 'home-info.json'");

		// Create json
		rapidjson::Document document = rapidjson::Document(rapidjson::kObjectType);

		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		// Create action list
		{
			rapidjson::Value actionListJson = rapidjson::Value(rapidjson::kArrayType);

			for (std::pair<uint32_t, Ref<Action>> pair : actionList)
			{
				rapidjson::Value actionJson = rapidjson::Value(rapidjson::kObjectType);

				pair.second->Save(actionJson, allocator);

				actionListJson.PushBack(actionJson, allocator);
			}

			document.AddMember("action-list", actionListJson, allocator);
		}

		// Create room list
		{
			rapidjson::Value roomListJson = rapidjson::Value(rapidjson::kArrayType);

			for (std::pair<uint32_t, Ref<Room>> pair : roomList)
			{
				rapidjson::Value roomJson = rapidjson::Value(rapidjson::kObjectType);

				pair.second->Save(roomJson, allocator);

				roomListJson.PushBack(roomJson, allocator);
			}

			document.AddMember("room-list", roomListJson, allocator);
		}

		// Create device list
		{
			rapidjson::Value deviceListJson = rapidjson::Value(rapidjson::kArrayType);

			for (std::pair<uint32_t, Ref<Device>> pair : deviceList)
			{
				rapidjson::Value deviceJson = rapidjson::Value(rapidjson::kObjectType);

				pair.second->Save(deviceJson, allocator);

				deviceListJson.PushBack(deviceJson, allocator);
			}

			document.AddMember("device-list", deviceListJson, allocator);
		}

		// Create device manager list
		{
			rapidjson::Value deviceManagerListJson = rapidjson::Value(rapidjson::kArrayType);

			for (std::pair<uint32_t, Ref<DeviceManager>> pair : deviceManagerList)
			{
				rapidjson::Value deviceJson = rapidjson::Value(rapidjson::kObjectType);

				pair.second->Save(deviceJson, allocator);

				deviceManagerListJson.PushBack(deviceJson, allocator);
			}

			document.AddMember("device-manager-list", deviceManagerListJson, allocator);
		}

		//Save to file
		FILE* file = fopen("home-info.json", "w");
		if (file == nullptr)
		{
			LOG_ERROR("Open/find 'home-info.json'");
			throw std::runtime_error("Open/find file 'home-info.json'");
		}

		char buffer[RAPIDJSON_BUFFER_SIZE_SMALL];
		rapidjson::FileWriteStream stream(file, buffer, sizeof(buffer));

		rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer = rapidjson::PrettyWriter<rapidjson::FileWriteStream>(stream);
		document.Accept(writer);

		fclose(file);
	}
}
