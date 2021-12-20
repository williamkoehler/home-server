#pragma once
#include "../../common.hpp"
#include "../mock/MockDeviceController.hpp"
#include "../mock/MockRoom.hpp"

struct DeviceControllerCache
{
	identifier_t id;
	std::string name;
	identifier_t pluginID;
	identifier_t roomID;
};
void TestTableSize(size_t tableSize)
{
	//! Check table size
	BOOST_CHECK_MESSAGE(database->GetDeviceControllerCount() == tableSize, "Device controller table has too many or too few elements");
}
void TestTableContent(const DeviceControllerCache** controllers, size_t controllerCount)
{
	size_t counter = 0;

	// Iterate over every element in the database
	BOOST_REQUIRE_MESSAGE(database->LoadDeviceControllers([&controllers, &controllerCount, &counter](identifier_t id, const std::string& name, identifier_t pluginID, identifier_t roomID, const std::string& data) -> void
		{
			if (counter < controllerCount)
			{
				const DeviceControllerCache* controller = controllers[counter];

				//! Check id
				BOOST_CHECK_MESSAGE(id == controller->id, "ID is not correct: '" << id << "' != '" << controller->id << "'");

				//! Check name
				BOOST_CHECK_MESSAGE(name == controller->name, "Name is not correct: '" << name << "' != '" << controller->name << "'");

				//! Check pluginID
				BOOST_CHECK_MESSAGE(pluginID == controller->pluginID, "PluginID is not correct: '" << pluginID << "' != '" << controller->pluginID << "'");

				//! Check roomID
				BOOST_CHECK_MESSAGE(roomID == controller->roomID, "RoomID is not correct: '" << roomID << "' != '" << controller->roomID << "'");
			}

			counter++;
		}), "Load device controllers from database");

	//! Compaire element count
	BOOST_CHECK_MESSAGE(counter <= controllerCount, "Device controller table has too many elements");
	BOOST_CHECK_MESSAGE(counter >= controllerCount, "Device controller table has too few elements");
}

#define UNKNOWN_CONTROLLER "unknown controller"

static DeviceControllerCache controller01 = { 1,  UNKNOWN_CONTROLLER, 0, 0 };
static DeviceControllerCache controller02 = { 2,  UNKNOWN_CONTROLLER, 0, 0 };
static DeviceControllerCache controller03 = { 3,  UNKNOWN_CONTROLLER, 0, 0 };
static DeviceControllerCache controller04 = { 4,  UNKNOWN_CONTROLLER, 0, 0 };
static DeviceControllerCache controller05 = { 5,  UNKNOWN_CONTROLLER, 0, 0 };
static DeviceControllerCache controller06 = { 6,  UNKNOWN_CONTROLLER, 0, 0 };
static DeviceControllerCache controller07 = { 7,  UNKNOWN_CONTROLLER, 0, 0 };
static DeviceControllerCache controller08 = { 8,  UNKNOWN_CONTROLLER, 0, 0 };
static DeviceControllerCache controller09 = { 9,  UNKNOWN_CONTROLLER, 0, 0 };
static DeviceControllerCache controller10 = { 10, UNKNOWN_CONTROLLER, 0, 0 };

void UpdateController(DeviceControllerCache* controllerCache, const std::string& name, identifier_t pluginID, identifier_t roomID)
{
	//! Update device cache
	controllerCache->name = name;
	controllerCache->pluginID = pluginID;
	controllerCache->roomID = roomID;

	//! Create room instance
	Ref<server::Room> room = roomID ? MockRoom(roomID) : nullptr;

	//! Create device instance
	Ref<server::DeviceController> controller = MockDeviceController(controllerCache->id, controllerCache->name, controllerCache->pluginID, room);
	BOOST_REQUIRE_MESSAGE(controller != nullptr, "Create server::DeviceController instance");

	BOOST_CHECK_MESSAGE(database->UpdateDeviceController(controller) == true, "Update controller '" << name << "'");
}
void UpdateControllerPropName(DeviceControllerCache* controllerCache, const std::string& name)
{
	//! Update name
	controllerCache->name = name;

	//! Create device instance
	Ref<server::DeviceController> controller = MockDeviceController(controllerCache->id, controllerCache->name);
	BOOST_REQUIRE_MESSAGE(controller != nullptr, "Create server::DeviceController instance");

	BOOST_CHECK_MESSAGE(database->UpdateDeviceControllerPropName(controller, "old name", name) == true, "Update device controller prop name");
}
void UpdateControllerPropRoom(DeviceControllerCache* controllerCache, identifier_t roomID)
{
	//! Update controller
	controllerCache->roomID = roomID;

	//! Create room instance
	Ref<server::Room> room = roomID ? MockRoom(roomID) : nullptr;

	//! Create device instance
	Ref<server::DeviceController> controller = MockDeviceController(controllerCache->id, controllerCache->name);
	BOOST_REQUIRE_MESSAGE(controller != nullptr, "Create server::DeviceController instance");

	BOOST_CHECK_MESSAGE(database->UpdateDeviceControllerPropRoom(controller, nullptr, room) == true, "Update device controller prop room id");
}