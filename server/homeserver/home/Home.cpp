#include "Home.hpp"
#include "../database/Database.hpp"
#include "Room.hpp"
#include "Device.hpp"
#include "DeviceController.hpp"
#include "../plugin/PluginManager.hpp"
#include <xxHash/xxhash.h>

namespace server
{
	boost::weak_ptr<Home> instanceHome;

	Home::Home()
	{
	}
	Home::~Home()
	{
		deviceList.clear();
		deviceControllerList.clear();
		roomList.clear();
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
			Ref<Database> database = Database::GetInstance();
			assert(database != nullptr);

			// Load rooms
			database->LoadRooms(
				boost::bind(&Home::LoadRoom, home, 
							boost::placeholders::_1, 
							boost::placeholders::_2, 
							boost::placeholders::_3));

			// Load device controllers
			database->LoadDeviceControllers(
				boost::bind(&Home::LoadDeviceController, home, 
							boost::placeholders::_1, 
							boost::placeholders::_2, 
							boost::placeholders::_3,
							boost::placeholders::_4,
							boost::placeholders::_5));

			// Load devices
			database->LoadDevices(
				boost::bind(&Home::LoadDevice, home,
							boost::placeholders::_1,
							boost::placeholders::_2,
							boost::placeholders::_3,
							boost::placeholders::_4,
							boost::placeholders::_5,
							boost::placeholders::_6));
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
	bool Home::LoadRoom(identifier_t roomID, const std::string& name, const std::string& type)
	{
		Ref<Room> room = boost::make_shared<Room>(name, roomID, type);

		if (room != nullptr)
		{
			roomList[roomID] = room;

			return true;
		}
		else
			return false;
	}
	Ref<Room> Home::AddRoom(const std::string& name, const std::string& type, rapidjson::Value& json)
	{
		boost::lock_guard lock(mutex);

		Ref<Database> database = Database::GetInstance();
		assert(database != nullptr);

		// Reserve room in database
		identifier_t roomID = database->ReserveRoom();
		if (roomID == 0)
			return nullptr;

		// Create new device
		Ref<Room> room = boost::make_shared<Room>(name, roomID, type);
		if (room == nullptr)
			return nullptr;

		if (room != nullptr)
		{
			if (!database->UpdateRoom(room))
				return nullptr;

			roomList[room->GetRoomID()] = room;
		}
		else
		{
			database->RemoveRoom(roomID);
			return nullptr;
		}

		UpdateTimestamp();

		return room;
	}
	Ref<Room> Home::GetRoom(identifier_t roomID)
	{
		boost::shared_lock_guard lock(mutex);

		const boost::unordered::unordered_map<identifier_t, Ref<Room>>::const_iterator it = roomList.find(roomID);
		if (it == roomList.end())
			return nullptr;

		return (*it).second;
	}
	bool Home::RemoveRoom(identifier_t roomID)
	{
		boost::lock_guard lock(mutex);

		if(roomList.erase(roomID))
		{
			Ref<Database> database = Database::GetInstance();
			assert(database != nullptr);

			database->RemoveRoom(roomID);

			return true;
		}
		else
		return false;
	}

	//! Device
	bool Home::LoadDevice(identifier_t deviceID, const std::string& name, identifier_t pluginID, identifier_t controllerID, identifier_t roomID, const std::string& data)
	{
		// Get device plugin
		Ref<PluginManager> pluginManager = PluginManager::GetInstance();
		assert(pluginManager != nullptr);

		Ref<home::DevicePlugin> plugin = pluginManager->CreateDevicePlugin(pluginID);
		if (plugin == nullptr)
		{
			LOG_ERROR("Create device plugin '{0}'", pluginID);
			return false;
		}

		// Get controller and room
		//! We don't care if no room nor controller is found, since it is allowed to be null
		Ref<DeviceController> controller = GetDeviceController(controllerID);
		Ref<Room> room = GetRoom(roomID);

		// Create new device
		Ref<Device> device = boost::make_shared<Device>(name, deviceID, plugin, controller, room);

		if (device != nullptr)
		{
			deviceList[deviceID] = device;

			return true;
		}
		else
			return false;
	}
	Ref<Device> Home::AddDevice(const std::string& name, identifier_t pluginID, identifier_t controllerID, identifier_t roomID, rapidjson::Value& json)
	{
		boost::lock_guard lock(mutex);

		Ref<Database> database = Database::GetInstance();
		assert(database != nullptr);

		// Reserve room in database
		identifier_t deviceID = database->ReserveDevice();
		if (deviceID == 0)
			return nullptr;

		// Get device plugin
		Ref<PluginManager> pluginManager = PluginManager::GetInstance();
		assert(pluginManager != nullptr);

		Ref<home::DevicePlugin> plugin = pluginManager->CreateDevicePlugin(pluginID);
		if (plugin == nullptr)
		{
			LOG_ERROR("Create device plugin '{0}'", pluginID);
			return nullptr;
		}

		// Get controller and room
		//! We don't care if no room nor controller is found, since it is allowed to be null
		Ref<DeviceController> controller = GetDeviceController(controllerID);
		Ref<Room> room = GetRoom(roomID);

		// Create new device
		Ref<Device> device = boost::make_shared<Device>(name, deviceID, plugin, controller, room);
		if (device == nullptr)
			return nullptr;

		if (device != nullptr)
		{
			//device->Load(json);

			if (!database->UpdateDevice(device))
				return nullptr;

			deviceList[device->GetDeviceID()] = device;
		}
		else
		{
			database->RemoveDevice(deviceID);
			return nullptr;
		}

		return device;
	}
	Ref<Device> Home::GetDevice(identifier_t deviceID)
	{
		boost::shared_lock_guard lock(mutex);

		const boost::unordered::unordered_map<identifier_t, Ref<Device>>::const_iterator it = deviceList.find(deviceID);
		if (it == deviceList.end())
			return nullptr;

		return (*it).second;
	}
	bool Home::RemoveDevice(identifier_t deviceID)
	{
		boost::lock_guard lock(mutex);

		if (deviceList.erase(deviceID))
		{
			Ref<Database> database = Database::GetInstance();
			assert(database != nullptr);

			database->RemoveDevice(deviceID);

			return true;
		}
		else
			return false;
	}

	//! DeviceController
	bool Home::LoadDeviceController(identifier_t controllerID, const std::string& name, identifier_t pluginID, identifier_t roomID, const std::string& data)
	{
		// Get device plugin
		Ref<PluginManager> pluginManager = PluginManager::GetInstance();
		assert(pluginManager != nullptr);

		Ref<home::DeviceControllerPlugin> plugin = pluginManager->CreateDeviceControllerPlugin(pluginID);
		if (plugin == nullptr)
		{
			LOG_ERROR("Create device controller plugin '{0}'", pluginID);
			return false;
		}

		// Get room
		//! We don't care if no controller is found, since it is allowed to be null
		Ref<Room> room = GetRoom(roomID);

		// Create new device controller
		Ref<DeviceController> controller = boost::make_shared<DeviceController>(name, controllerID, plugin, room);

		if (controller != nullptr)
		{
			deviceControllerList[controllerID] = controller;

			return true;
		}
		else
			return false;
	}
	Ref<DeviceController> Home::AddDeviceController(const std::string& name, identifier_t pluginID, identifier_t roomID, rapidjson::Value& json)
	{
		boost::lock_guard lock(mutex);

		Ref<Database> database = Database::GetInstance();
		assert(database != nullptr);

		// Reserve room in database
		identifier_t controllerID = database->ReserveDeviceController();
		if (controllerID == 0)
			return nullptr;

		// Get device plugin
		Ref<PluginManager> pluginManager = PluginManager::GetInstance();
		assert(pluginManager != nullptr);

		Ref<home::DeviceControllerPlugin> plugin = pluginManager->CreateDeviceControllerPlugin(pluginID);
		if (plugin == nullptr)
		{
			LOG_ERROR("Create device controller plugin '{0}'", pluginID);
			return nullptr;
		}

		// Get room
		//! We don't care if no controller is found, since it is allowed to be null
		Ref<Room> room = GetRoom(roomID);

		// Create new device controller
		Ref<DeviceController> controller = boost::make_shared<DeviceController>(name, controllerID, plugin, room);

		if (controller != nullptr)
		{
			//controller->Load(json);

			if (!database->UpdateDeviceController(controller))
				return nullptr;

			deviceControllerList[controllerID] = controller;
		}
		else
		{
			database->RemoveDeviceController(controllerID);
			return nullptr;
		}

		return controller;
	}
	Ref<DeviceController> Home::GetDeviceController(identifier_t controllerID)
	{
		boost::shared_lock_guard lock(mutex);

		const boost::unordered::unordered_map<identifier_t, Ref<DeviceController>>::const_iterator it = deviceControllerList.find(controllerID);
		if (it == deviceControllerList.end())
			return nullptr;

		return (*it).second;
	}
	bool Home::RemoveDeviceController(identifier_t controllerID)
	{
		boost::lock_guard lock(mutex);

		if (deviceControllerList.erase(controllerID))
		{
			Ref<Database> database = Database::GetInstance();
			assert(database != nullptr);

			database->RemoveDeviceController(controllerID);

			return true;
		}
		else
			return false;
	}

	//! Worker
	void Home::Worker()
	{
		//TODO Work
	}
}
