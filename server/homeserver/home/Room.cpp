#include "Room.hpp"
#include "../database/Database.hpp"
#include "Home.hpp"
#include "Device.hpp"
#include "DeviceController.hpp"
#include "../plugin/PluginManager.hpp"
#include <xxHash/xxhash.h>

namespace server
{
	Room::Room(const std::string& name, identifier_t roomID, const std::string& type)
		: name(name), roomID(roomID), type(type)
	{ 	}
	Room::~Room()
	{
		deviceList.clear();
		deviceControllerList.clear();
	}

	std::string Room::GetName()
	{
		boost::shared_lock_guard lock(mutex);
		return name;
	}
	bool Room::SetName(const std::string& v)
	{
		boost::lock_guard lock(mutex);
		if (Database::GetInstance()->UpdateRoomPropName(shared_from_this(), name, v))
		{
			name = v;
			return true;
		}
		else
			return false;
	}

	std::string Room::GetType()
	{
		boost::shared_lock_guard lock(mutex);
		return type;
	}
	bool Room::SetType(const std::string& v)
	{
		boost::lock_guard lock(mutex);
		if (Database::GetInstance()->UpdateRoomPropType(shared_from_this(), type, v))
		{
			type = v;
			return true;
		}
		else
			return false;
	}

	bool Room::AddDevice(Ref<Device> device)
	{
		assert(device != nullptr);

		boost::lock_guard lock(mutex);

		// Add device id to list
		return deviceList.insert(device->GetDeviceID()).second;
	}
	bool Room::RemoveDevice(Ref<Device> device)
	{
		assert(device != nullptr);

		boost::lock_guard lock(mutex);

		// Remove device id from list
		return deviceList.erase(device->GetDeviceID());
	}

	bool Room::AddDeviceController(Ref<DeviceController> controller)
	{
		assert(controller != nullptr);

		boost::lock_guard lock(mutex);

		// Add device controller id to list
		return deviceControllerList.insert(controller->GetDeviceControllerID()).second;
	}
	bool Room::RemoveDeviceController(Ref<DeviceController> controller)
	{
		assert(controller != nullptr);

		boost::lock_guard lock(mutex);

		// Remove device id from list
		return deviceControllerList.erase(controller->GetDeviceControllerID());
	}
}