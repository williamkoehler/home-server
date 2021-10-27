#include "Room.hpp"
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