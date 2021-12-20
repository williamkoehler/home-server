#include "Home.hpp"
#include "../database/Database.hpp"
#include "../Core.hpp"
#include "Room.hpp"
#include "Device.hpp"
#include "DeviceController.hpp"
#include "Action.hpp"
#include "../plugin/PluginManager.hpp"
#include "../scripting/ScriptManager.hpp"
#include <xxhash/xxhash.h>

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
			// Start service
			home->service = boost::make_shared<boost::asio::io_service>(1);

			// Load from database
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
		Ref<Room> room = Room::Create(name, roomID, type);

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
		Ref<Database> database = Database::GetInstance();
		assert(database != nullptr);

		// Reserve room in database
		identifier_t roomID = database->ReserveRoom();
		if (roomID == 0)
			return nullptr;

		// Create new device
		Ref<Room> room = Room::Create(name, roomID, type);
		if (room == nullptr)
			return nullptr;

		if (room != nullptr)
		{
			if (!database->UpdateRoom(room))
				return nullptr;

			boost::lock_guard lock(mutex);
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

		const robin_hood::unordered_node_map<identifier_t, Ref<Room>>::const_iterator it = roomList.find(roomID);
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
			LOG_ERROR("Creating device plugin '{0}'", pluginID);
			return false;
		}

		// Get controller and room
		//! We don't care if no room nor controller is found, since it is allowed to be null
		Ref<DeviceController> controller = GetDeviceController(controllerID);
		Ref<Room> room = GetRoom(roomID);

		// Create new device
		Ref<Device> device = Device::Create(name, deviceID, std::move(plugin), std::move(controller), std::move(room));

		if (device != nullptr)
		{
			deviceList[deviceID] = device;

			// Initialize device plugin
			device->Initialize();

			return true;
		}
		else
			return false;
	}
	Ref<Device> Home::AddDevice(const std::string& name, identifier_t pluginID, identifier_t controllerID, identifier_t roomID, rapidjson::Value& json)
	{
		Ref<Database> database = Database::GetInstance();
		assert(database != nullptr);

		// Get device plugin
		Ref<PluginManager> pluginManager = PluginManager::GetInstance();
		assert(pluginManager != nullptr);

		Ref<home::DevicePlugin> plugin = pluginManager->CreateDevicePlugin(pluginID);
		if (plugin == nullptr)
		{
			LOG_ERROR("Creating device plugin '{0}'", pluginID);
			return nullptr;
		}

		// Reserve room in database
		identifier_t deviceID = database->ReserveDevice();
		if (deviceID == 0)
			return nullptr;

		// Get controller and room
		//! We don't care if no room nor controller is found, since it is allowed to be null
		Ref<DeviceController> controller = GetDeviceController(controllerID);
		Ref<Room> room = GetRoom(roomID);

		// Create new device
		Ref<Device> device = Device::Create(name, deviceID, std::move(plugin), std::move(controller), std::move(room));
		if (device == nullptr)
			return nullptr;

		if (device != nullptr)
		{
			if (!database->UpdateDevice(device))
				return nullptr;

			boost::lock_guard lock(mutex);
			deviceList[device->GetDeviceID()] = device;

			// Initialize device plugin
			device->Initialize();
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

		const robin_hood::unordered_node_map<identifier_t, Ref<Device>>::const_iterator it = deviceList.find(deviceID);
		if (it == deviceList.end())
			return nullptr;

		return (*it).second;
	}
	bool Home::RemoveDevice(identifier_t deviceID)
	{
		boost::lock_guard lock(mutex);

		const robin_hood::unordered_node_map<identifier_t, Ref<Device>>::const_iterator it = deviceList.find(deviceID);
		if (it != deviceList.end())
		{
			Ref<Database> database = Database::GetInstance();
			assert(database != nullptr);

			database->RemoveDevice(deviceID);

			// Terminate device
			it->second->Terminate();

			deviceList.erase(it);

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
			LOG_ERROR("Creating device controller plugin '{0}'", pluginID);
			return false;
		}

		// Get room
		//! We don't care if no controller is found, since it is allowed to be null
		Ref<Room> room = GetRoom(roomID);

		// Create new device controller
		Ref<DeviceController> controller = DeviceController::Create(name, controllerID, std::move(plugin), std::move(room));

		if (controller != nullptr)
		{
			deviceControllerList[controllerID] = controller;

			// Initialize device controller plugin
			controller->Initialize();

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

		// Get device plugin
		Ref<PluginManager> pluginManager = PluginManager::GetInstance();
		assert(pluginManager != nullptr);

		Ref<home::DeviceControllerPlugin> plugin = pluginManager->CreateDeviceControllerPlugin(pluginID);
		if (plugin == nullptr)
		{
			LOG_ERROR("Creating device controller plugin '{0}'", pluginID);
			return nullptr;
		}

		// Reserve device controller in database
		identifier_t controllerID = database->ReserveDeviceController();
		if (controllerID == 0)
			return nullptr;

		// Get room
		//! We don't care if no room is found, since it is allowed to be null
		Ref<Room> room = GetRoom(roomID);

		// Create new device controller
		Ref<DeviceController> controller = DeviceController::Create(name, controllerID, std::move(plugin), std::move(room));

		if (controller != nullptr)
		{
			if (!database->UpdateDeviceController(controller))
				return nullptr;

			boost::lock_guard lock(mutex);
			deviceControllerList[controllerID] = controller;

			// Initialie device controller plugin
			controller->Initialize();
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

		const robin_hood::unordered_node_map<identifier_t, Ref<DeviceController>>::const_iterator it = deviceControllerList.find(controllerID);
		if (it == deviceControllerList.end())
			return nullptr;

		return (*it).second;
	}
	bool Home::RemoveDeviceController(identifier_t controllerID)
	{
		boost::lock_guard lock(mutex);

		const robin_hood::unordered_node_map<identifier_t, Ref<DeviceController>>::const_iterator it = deviceControllerList.find(controllerID);
		if (it != deviceControllerList.end())
		{
			Ref<Database> database = Database::GetInstance();
			assert(database != nullptr);

			database->RemoveDeviceController(controllerID);

			// Terminate device controller plugin
			it->second->Terminate();

			deviceControllerList.erase(it);

			return true;
		}
		else
			return false;
	}

	//! Action
	bool Home::LoadAction(identifier_t actionID, const std::string& name, identifier_t sourceID, identifier_t roomID, const std::string& data)
	{
		// Get script
		Ref<ScriptManager> scriptManager = ScriptManager::GetInstance();
		assert(scriptManager != nullptr);

		Ref<Script> script = scriptManager->CreateActionScript(sourceID);
		if (script == nullptr)
		{
			LOG_ERROR("Creating action script '{0}'", sourceID);
			return false;
		}

		// Get room
		//! We don't care if no room is found, since it is allowed to be null
		Ref<Room> room = GetRoom(roomID);

		// Create new action
		Ref<Action> action = Action::Create(name, actionID, std::move(script), std::move(room));

		if (action != nullptr)
		{
			actionList[actionID] = action;

			return true;
		}
		else
			return false;
	}
	Ref<Action> Home::AddAction(const std::string& name, identifier_t sourceID, identifier_t roomID, rapidjson::Value& json)
	{
		boost::lock_guard lock(mutex);

		Ref<Database> database = Database::GetInstance();
		assert(database != nullptr);

		// Get script
		Ref<ScriptManager> scriptManager = ScriptManager::GetInstance();
		assert(scriptManager != nullptr);

		Ref<Script> script = scriptManager->CreateActionScript(sourceID);
		if (script == nullptr)
		{
			LOG_ERROR("Creating action script '{0}'", sourceID);
			return nullptr;
		}

		// Reserve action in database
		identifier_t actionID = database->ReserveAction();
		if (actionID == 0)
			return nullptr;

		// Get room
		//! We don't care if no room is found, since it is allowed to be null
		Ref<Room> room = GetRoom(roomID);

		// Create new action
		Ref<Action> action = Action::Create(name, actionID, std::move(script), std::move(room));

		if (action != nullptr)
		{
			if (!database->UpdateAction(action))
				return nullptr;

			boost::lock_guard lock(mutex);
			actionList[actionID] = action;
		}
		else
		{
			database->RemoveAction(actionID);
			return nullptr;
		}

		return action;
	}
	Ref<Action> Home::GetAction(identifier_t actionID)
	{
		boost::shared_lock_guard lock(mutex);

		const robin_hood::unordered_node_map<identifier_t, Ref<Action>>::const_iterator it = actionList.find(actionID);
		if (it == actionList.end())
			return nullptr;

		return (*it).second;
	}
	bool Home::RemoveAction(identifier_t actionID)
	{
		boost::lock_guard lock(mutex);

		if (actionList.erase(actionID))
		{
			Ref<Database> database = Database::GetInstance();
			assert(database != nullptr);

			database->RemoveAction(actionID);

			return true;
		}
		else
			return false;
	}

	//! Worker
	void Home::Worker()
	{
		Ref<Core> core = Core::GetInstance();

		while (core->IsRunning())
		{
			try
			{
				service->run();
				boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
			}
			catch (std::exception e)
			{
				printf("Ooops!!! Something bad happend");
				LOG_ERROR("An exception was thrown and not catched: {0}", e.what());
			}
		}
	}

	void Home::Run() 
	{
		// Start worker thread
		worker = boost::thread(boost::bind(&Home::Worker, shared_from_this()));
	}
}
