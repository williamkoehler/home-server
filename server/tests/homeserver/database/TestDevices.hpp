#pragma once
#include "../../common.hpp"
#include "../mock/MockDevice.hpp"
#include "../mock/MockRoom.hpp"
#include "../mock/MockDeviceController.hpp"

struct DeviceCache
{
	identifier_t id;
	std::string name;
	identifier_t pluginID;
	identifier_t controllerID;
	identifier_t roomID;
};
void TestTableSize(size_t tableSize)
{
	//! Check table size
	BOOST_CHECK_MESSAGE(database->GetDeviceCount() == tableSize, "Device table has too many or too few elements");
}
void TestTableContent(const DeviceCache** devices, size_t deviceCount)
{
	size_t counter = 0;

	// Iterate over every element in the database
	BOOST_REQUIRE_MESSAGE(database->LoadDevices([&devices, &deviceCount, &counter](identifier_t id, const std::string& name, identifier_t pluginID, identifier_t controllerID, identifier_t roomID, const std::string& data) -> void
		{
			if (counter < deviceCount)
			{
				const DeviceCache* device = devices[counter];

				//! Check id
				BOOST_CHECK_MESSAGE(id == device->id, "ID is not correct: '" << id << "' != '" << device->id << "'");

				//! Check name
				BOOST_CHECK_MESSAGE(name == device->name, "Name is not correct: '" << name << "' != '" << device->name << "'");

				//! Check pluginID
				BOOST_CHECK_MESSAGE(pluginID == device->pluginID, "PluginID is not correct: '" << pluginID << "' != '" << device->pluginID << "'");

				//! Check controllerID
				BOOST_CHECK_MESSAGE(controllerID == device->controllerID, "ControllerID is not correct: '" << controllerID << "' != '" << device->controllerID << "'");

				//! Check roomID
				BOOST_CHECK_MESSAGE(roomID == device->roomID, "RoomID is not correct: '" << roomID << "' != '" << device->roomID << "'");
			}

			counter++;
		}), "Load devices from database");

	//! Compaire element count
	BOOST_CHECK_MESSAGE(counter <= deviceCount, "Device table has too many elements");
	BOOST_CHECK_MESSAGE(counter >= deviceCount, "Device table has too few elements");
}

#define UNKNOWN_DEVICE "unknown device"

static DeviceCache device01 = { 1,  UNKNOWN_DEVICE, 0, 0, 0 };
static DeviceCache device02 = { 2,  UNKNOWN_DEVICE, 0, 0, 0 };
static DeviceCache device03 = { 3,  UNKNOWN_DEVICE, 0, 0, 0 };
static DeviceCache device04 = { 4,  UNKNOWN_DEVICE, 0, 0, 0 };
static DeviceCache device05 = { 5,  UNKNOWN_DEVICE, 0, 0, 0 };
static DeviceCache device06 = { 6,  UNKNOWN_DEVICE, 0, 0, 0 };
static DeviceCache device07 = { 7,  UNKNOWN_DEVICE, 0, 0, 0 };
static DeviceCache device08 = { 8,  UNKNOWN_DEVICE, 0, 0, 0 };
static DeviceCache device09 = { 9,  UNKNOWN_DEVICE, 0, 0, 0 };
static DeviceCache device10 = { 10, UNKNOWN_DEVICE, 0, 0, 0 };

void UpdateDevice(DeviceCache* deviceCache, const std::string& name, identifier_t pluginID, identifier_t controllerID, identifier_t roomID)
{
	//! Update device cache
	deviceCache->name = name;
	deviceCache->pluginID = pluginID;
	deviceCache->controllerID = controllerID;
	deviceCache->roomID = roomID;

	//! Create device controller instance
	Ref<server::DeviceController> controller = controllerID ? MockDeviceController(controllerID) : nullptr;

	//! Create room instance
	Ref<server::Room> room = roomID ? MockRoom(roomID) : nullptr;

	//! Create device instance
	Ref<server::Device> device = MockDevice(deviceCache->id, deviceCache->name, deviceCache->pluginID, controller, room);
	BOOST_REQUIRE_MESSAGE(device != nullptr, "Create server::Device instance");

	BOOST_CHECK_MESSAGE(database->UpdateDevice(device) == true, "Update device '" << name << "'");
}
void UpdateDevicePropName(DeviceCache* deviceCache, const std::string& name)
{
	//! Update name
	deviceCache->name = name;

	//! Create device instance
	Ref<server::Device> device = MockDevice(deviceCache->id, deviceCache->name);
	BOOST_REQUIRE_MESSAGE(device != nullptr, "Create server::Device instance");

	BOOST_CHECK_MESSAGE(database->UpdateDevicePropName(device, "old name", name) == true, "Update device prop name");
}
void UpdateDevicePropController(DeviceCache* deviceCache, identifier_t controllerID)
{
	//! Update controller
	deviceCache->controllerID = controllerID;

	//! Create device controller instance
	Ref<server::DeviceController> controller = controllerID ? MockDeviceController(controllerID) : nullptr;

	//! Create device instance
	Ref<server::Device> device = MockDevice(deviceCache->id, deviceCache->name);
	BOOST_REQUIRE_MESSAGE(device != nullptr, "Create server::Device instance");

	BOOST_CHECK_MESSAGE(database->UpdateDevicePropDeviceController(device, nullptr, controller) == true, "Update device prop controller id");
}
void UpdateDevicePropRoom(DeviceCache* deviceCache, identifier_t roomID)
{
	//! Update controller
	deviceCache->roomID = roomID;

	//! Create room instance
	Ref<server::Room> room = roomID ? MockRoom(roomID) : nullptr;

	//! Create device instance
	Ref<server::Device> device = MockDevice(deviceCache->id, deviceCache->name);
	BOOST_REQUIRE_MESSAGE(device != nullptr, "Create server::Device instance");

	BOOST_CHECK_MESSAGE(database->UpdateDevicePropRoom(device, nullptr, room) == true, "Update device prop room id");
}