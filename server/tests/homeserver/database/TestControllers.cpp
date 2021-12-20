#include "TestDatabase.hpp"
#include "TestControllers.hpp"
#include "../mock/MockString.hpp"

BOOST_AUTO_TEST_CASE(check_initial_state)
{
	BOOST_REQUIRE(database != nullptr);

	// Check initial table state
	TestTableSize(0);
	TestTableContent(nullptr, 0);
}

//! Test reserve
BOOST_AUTO_TEST_CASE(test_reserve_device_controller)
{
	// Insert 10 different devices
	database->ReserveDeviceController();
	database->ReserveDeviceController();
	database->ReserveDeviceController();
	database->ReserveDeviceController();
	database->ReserveDeviceController();

	database->ReserveDeviceController();
	database->ReserveDeviceController();
	database->ReserveDeviceController();
	database->ReserveDeviceController();
	database->ReserveDeviceController();

	// Check table
	TestTableContent(nullptr, 0);

	// Check table size
	TestTableSize(10);
}

//! Test update
BOOST_AUTO_TEST_CASE(test_partial_update_device_controller_1)
{
	static const DeviceControllerCache* controllers[] = {
		&controller01,
		&controller02,
		&controller04,
		&controller05,
		&controller09,
	};

	// Update 5 devices
	UpdateController(&controller01, "eciveD rellortnoC 10", 484, 0);
	UpdateController(&controller02, "eciveD rellortnoC 20", 212, 6);
	UpdateController(&controller04, "eciveD rellortnoC 40", 242, 5);
	UpdateController(&controller05, "eciveD rellortnoC 50", 532, 7);
	UpdateController(&controller09, "eciveD rellortnoC 90", 654, 0);

	// Check table
	TestTableContent(controllers, sizeof(controllers) / sizeof(DeviceControllerCache*));

	// Check table size
	TestTableSize(10);
}
BOOST_AUTO_TEST_CASE(test_partial_update_device_controller_2)
{
	static const DeviceControllerCache* controllers[] = {
		&controller01,
		&controller02,
		&controller03,
		&controller04,
		&controller05,

		&controller06,
		&controller07,
		&controller08,
		&controller09,
		&controller10,
	};

	// Update 5 devices
	UpdateController(&controller03, "Device Controller 03", 543, 1685);
	UpdateController(&controller06, "Device Controller 06", 251, 2113);
	UpdateController(&controller07, "Device Controller 07", 224, 2551);
	UpdateController(&controller08, "Device Controller 08", 242, 2155);
	UpdateController(&controller10, "Device Controller 10", 613, 2225);

	// Check table
	TestTableContent(controllers, sizeof(controllers) / sizeof(DeviceControllerCache*));

	// Re-Update 5 Devices
	UpdateController(&controller01, "Device Controller 01", 254, 2432);
	UpdateController(&controller02, "Device Controller 02", 254, 5121);
	UpdateController(&controller04, "Device Controller 04", 254, 2221);
	UpdateController(&controller05, "Device Controller 05", 254, 2135);
	UpdateController(&controller09, "Device Controller 09", 254, 3505);

	// Check table
	TestTableContent(controllers, sizeof(controllers) / sizeof(DeviceControllerCache*));

	// Check table size
	TestTableSize(10);
}

//! Test update prop
BOOST_AUTO_TEST_CASE(test_update_device_controller_prop)
{
	static const DeviceControllerCache* controllers[] = {
		&controller01,
		&controller02,
		&controller03,
		&controller04,
		&controller05,

		&controller06,
		&controller07,
		&controller08,
		&controller09,
		&controller10,
	};

	UpdateControllerPropName(&controller01, "Updated-Device 1");
	UpdateControllerPropName(&controller02, "Updated-Device 2");
	UpdateControllerPropName(&controller05, "This is just a long device name to check if very very long text is working correctly");
	UpdateControllerPropName(&controller02, "Device 2 is a very good name");

	UpdateControllerPropRoom(&controller09, 2);
	UpdateControllerPropRoom(&controller07, 5);
	UpdateControllerPropRoom(&controller05, 454654684685004);
	UpdateControllerPropRoom(&controller02, 245641350023513);

	// Check table
	TestTableContent(controllers, sizeof(controllers) / sizeof(DeviceControllerCache*));

	// Check table size
	TestTableSize(10);
}

//! Test update 2
BOOST_AUTO_TEST_CASE(test_update_device_controller)
{
	static const DeviceControllerCache* controllers[] = {
		&controller01,
		&controller02,
		&controller03,
		&controller04,
		&controller05,

		&controller06,
		&controller07,
		&controller08,
		&controller09,
		&controller10,
	};

	// Update all devices
	UpdateController(&controller01, "Device 01", 468, 2);
	UpdateController(&controller02, "Device 02", 531, 5);
	UpdateController(&controller03, "Device 03", 212, 0);
	UpdateController(&controller04, "Device 04", 234, 5);
	UpdateController(&controller05, "Device 05", 254, 5);
	UpdateController(&controller06, "Device 06", 584, 4);
	UpdateController(&controller07, "Device 07", 254, 3);
	UpdateController(&controller08, "Device 08", 343, 4);
	UpdateController(&controller09, "Device 09", 531, 0);
	UpdateController(&controller10, "Device 10", 243, 0);

	// Check table
	TestTableContent(controllers, sizeof(controllers) / sizeof(DeviceControllerCache*));

	// Check table size
	TestTableSize(10);
}

//! Test remove
BOOST_AUTO_TEST_CASE(test_partial_remove_device_controller_1)
{
	static const DeviceControllerCache* controllers[] = {
		&controller03,
		&controller06,
		&controller07,
		&controller08,
		&controller10,
	};

	// Remove 5 devices
	BOOST_CHECK_MESSAGE(database->RemoveDeviceController(4) == true, "Remove device controller  4");
	BOOST_CHECK_MESSAGE(database->RemoveDeviceController(2) == true, "Remove device controller  2");
	BOOST_CHECK_MESSAGE(database->RemoveDeviceController(1) == true, "Remove device controller  1");
	BOOST_CHECK_MESSAGE(database->RemoveDeviceController(5) == true, "Remove device controller  5");
	BOOST_CHECK_MESSAGE(database->RemoveDeviceController(9) == true, "Remove device controller  9");

	// Remove 2 devices twice
	BOOST_CHECK_MESSAGE(database->RemoveDeviceController(2) == true, "Remove device controller  2 twice");
	BOOST_CHECK_MESSAGE(database->RemoveDeviceController(4) == true, "Remove device controller  4 twice");

	// Check table
	TestTableContent(controllers, sizeof(controllers) / sizeof(DeviceControllerCache*));

	// Check table size
	TestTableSize(5);
}
BOOST_AUTO_TEST_CASE(test_partial_remove_device_2)
{
	// Remove 5 devices
	BOOST_CHECK_MESSAGE(database->RemoveDeviceController(3) == true, "Remove device  3");
	BOOST_CHECK_MESSAGE(database->RemoveDeviceController(6) == true, "Remove device  6");
	BOOST_CHECK_MESSAGE(database->RemoveDeviceController(7) == true, "Remove device  7");
	BOOST_CHECK_MESSAGE(database->RemoveDeviceController(8) == true, "Remove device  8");
	BOOST_CHECK_MESSAGE(database->RemoveDeviceController(10) == true, "Remove device 10");

	// Check table
	TestTableContent(nullptr, 0);

	// Remove 3 devices twice
	BOOST_CHECK_MESSAGE(database->RemoveDeviceController(7) == true, "Remove device  7 twice");
	BOOST_CHECK_MESSAGE(database->RemoveDeviceController(6) == true, "Remove device  6 twice");
	BOOST_CHECK_MESSAGE(database->RemoveDeviceController(3) == true, "Remove device  3 twice");

	// Check table
	TestTableContent(nullptr, 0);

	// Check table size
	TestTableSize(0);
}

#define MAGIC_NUMBER (0x4DA45D1C76F4A969)

//! Big scale test
BOOST_AUTO_TEST_CASE(test_big_scale)
{
	static const size_t testSize = 2000;

	DeviceControllerCache controllerList[testSize];
	DeviceControllerCache* controllers[testSize];

	//! Fill database
	for (size_t index = 0; index < testSize; index++)
	{
		DeviceControllerCache& controllerCache = controllerList[index];
		controllerCache.id = database->ReserveDevice();
		controllerCache.name = GenerateString(index % 256, index * 10);
		controllerCache.pluginID = MAGIC_NUMBER + index * 6;
		controllerCache.roomID = MAGIC_NUMBER + index * 5;

		//! Create room instance
		Ref<server::Room> room = controllerCache.roomID ? MockRoom(controllerCache.roomID) : nullptr;

		Ref<server::DeviceController> controller = MockDeviceController(
			controllerCache.id,
			controllerCache.name,
			controllerCache.pluginID,
			room);
		database->UpdateDeviceController(controller);

		controllers[index] = &controllerCache;
	}

	//! Verify database

	// Check table
	TestTableContent((const DeviceControllerCache**)controllers, testSize);

	// Check table size
	TestTableSize(testSize);
}