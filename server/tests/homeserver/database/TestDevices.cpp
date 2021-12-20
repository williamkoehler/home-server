#include "TestDatabase.hpp"
#include "TestDevices.hpp"
#include "../mock/MockString.hpp"

BOOST_AUTO_TEST_CASE(check_initial_state)
{
	BOOST_REQUIRE(database != nullptr);

	// Check initial table state
	TestTableSize(0);
	TestTableContent(nullptr, 0);
}

//! Test reserve
BOOST_AUTO_TEST_CASE(test_reserve_device)
{
	// Insert 10 different devices
	database->ReserveDevice();
	database->ReserveDevice();
	database->ReserveDevice();
	database->ReserveDevice();
	database->ReserveDevice();

	database->ReserveDevice();
	database->ReserveDevice();
	database->ReserveDevice();
	database->ReserveDevice();
	database->ReserveDevice();

	// Check table
	TestTableContent(nullptr, 0);

	// Check table size
	TestTableSize(10);
}

//! Test update
BOOST_AUTO_TEST_CASE(test_partial_update_device_1)
{
	static const DeviceCache* devices[] = {
		&device01,
		&device02,
		&device04,
		&device05,
		&device09,
	};

	// Update 5 devices
	UpdateDevice(&device01, "eciveD 10", 484, 0, 0);
	UpdateDevice(&device02, "eciveD 20", 212, 6, 4);
	UpdateDevice(&device04, "eciveD 40", 242, 5, 5);
	UpdateDevice(&device05, "eciveD 50", 532, 1, 5);
	UpdateDevice(&device09, "eciveD 90", 654, 0, 0);

	// Check table
	TestTableContent(devices, sizeof(devices) / sizeof(DeviceCache*));

	// Check table size
	TestTableSize(10);
}
BOOST_AUTO_TEST_CASE(test_partial_update_device_2)
{
	static const DeviceCache* devices[] = {
		&device01,
		&device02,
		&device03,
		&device04,
		&device05,

		&device06,
		&device07,
		&device08,
		&device09,
		&device10,
	};

	// Update 5 devices
	UpdateDevice(&device03, "Device 03", 543, 1685, 1424);
	UpdateDevice(&device06, "Device 06", 251, 5111, 2113);
	UpdateDevice(&device07, "Device 07", 224, 5185, 2551);
	UpdateDevice(&device08, "Device 08", 242, 2155, 2412);
	UpdateDevice(&device10, "Device 10", 613, 2225, 5143);

	// Check table
	TestTableContent(devices, sizeof(devices) / sizeof(DeviceCache*));

	// Re-Update 5 Devices
	UpdateDevice(&device01, "Device 01", 254, 5351, 2432);
	UpdateDevice(&device02, "Device 02", 254, 1210, 5121);
	UpdateDevice(&device04, "Device 04", 254, 5313, 2221);
	UpdateDevice(&device05, "Device 05", 254, 3224, 2135);
	UpdateDevice(&device09, "Device 09", 254, 3505, 3153);

	// Check table
	TestTableContent(devices, sizeof(devices) / sizeof(DeviceCache*));

	// Check table size
	TestTableSize(10);
}

//! Test update prop
BOOST_AUTO_TEST_CASE(test_update_device_prop)
{
	static const DeviceCache* devices[] = {
		&device01,
		&device02,
		&device03,
		&device04,
		&device05,

		&device06,
		&device07,
		&device08,
		&device09,
		&device10,
	};

	UpdateDevicePropName(&device01, "Updated-Device 1");
	UpdateDevicePropName(&device02, "Updated-Device 2");
	UpdateDevicePropName(&device05, "This is just a long device name to check if very very long text is working correctly");
	UpdateDevicePropName(&device02, "Device 2 is a very good name");

	UpdateDevicePropController(&device10, 5);
	UpdateDevicePropController(&device07, 4);
	UpdateDevicePropController(&device09, 521525125754647984);
	UpdateDevicePropController(&device10, 223545464005893985);

	UpdateDevicePropRoom(&device09, 2);
	UpdateDevicePropRoom(&device07, 5);
	UpdateDevicePropRoom(&device05, 454654684685004);
	UpdateDevicePropRoom(&device02, 245641350023513);

	// Check table
	TestTableContent(devices, sizeof(devices) / sizeof(DeviceCache*));

	// Check table size
	TestTableSize(10);
}

//! Test update 2
BOOST_AUTO_TEST_CASE(test_update_device)
{
	static const DeviceCache* devices[] = {
		&device01,
		&device02,
		&device03,
		&device04,
		&device05,

		&device06,
		&device07,
		&device08,
		&device09,
		&device10,
	};

	// Update all devices
	UpdateDevice(&device01, "Device 01", 468, 1, 2);
	UpdateDevice(&device02, "Device 02", 531, 1, 5);
	UpdateDevice(&device03, "Device 03", 212, 5, 0);
	UpdateDevice(&device04, "Device 04", 234, 4, 5);
	UpdateDevice(&device05, "Device 05", 254, 5, 5);
	UpdateDevice(&device06, "Device 06", 584, 8, 4);
	UpdateDevice(&device07, "Device 07", 254, 7, 3);
	UpdateDevice(&device08, "Device 08", 343, 9, 4);
	UpdateDevice(&device09, "Device 09", 531, 5, 0);
	UpdateDevice(&device10, "Device 10", 243, 0, 0);

	// Check table
	TestTableContent(devices, sizeof(devices) / sizeof(DeviceCache*));

	// Check table size
	TestTableSize(10);
}

//! Test remove
BOOST_AUTO_TEST_CASE(test_partial_remove_device_1)
{
	static const DeviceCache* devices[] = {
		&device03,
		&device06,
		&device07,
		&device08,
		&device10,
	};

	// Remove 5 devices
	BOOST_CHECK_MESSAGE(database->RemoveDevice(4) == true, "Remove device  4");
	BOOST_CHECK_MESSAGE(database->RemoveDevice(2) == true, "Remove device  2");
	BOOST_CHECK_MESSAGE(database->RemoveDevice(1) == true, "Remove device  1");
	BOOST_CHECK_MESSAGE(database->RemoveDevice(5) == true, "Remove device  5");
	BOOST_CHECK_MESSAGE(database->RemoveDevice(9) == true, "Remove device  9");

	// Remove 2 devices twice
	BOOST_CHECK_MESSAGE(database->RemoveDevice(2) == true, "Remove device  2 twice");
	BOOST_CHECK_MESSAGE(database->RemoveDevice(4) == true, "Remove device  4 twice");

	// Check table
	TestTableContent(devices, sizeof(devices) / sizeof(DeviceCache*));

	// Check table size
	TestTableSize(5);
}
BOOST_AUTO_TEST_CASE(test_partial_remove_device_2)
{
	// Remove 5 devices
	BOOST_CHECK_MESSAGE(database->RemoveDevice(3) == true, "Remove device  3");
	BOOST_CHECK_MESSAGE(database->RemoveDevice(6) == true, "Remove device  6");
	BOOST_CHECK_MESSAGE(database->RemoveDevice(7) == true, "Remove device  7");
	BOOST_CHECK_MESSAGE(database->RemoveDevice(8) == true, "Remove device  8");
	BOOST_CHECK_MESSAGE(database->RemoveDevice(10) == true, "Remove device 10");

	// Check table
	TestTableContent(nullptr, 0);

	// Remove 3 devices twice
	BOOST_CHECK_MESSAGE(database->RemoveDevice(7) == true, "Remove device  7 twice");
	BOOST_CHECK_MESSAGE(database->RemoveDevice(6) == true, "Remove device  6 twice");
	BOOST_CHECK_MESSAGE(database->RemoveDevice(3) == true, "Remove device  3 twice");

	// Check table
	TestTableContent(nullptr, 0);

	// Check table size
	TestTableSize(0);
}

#define MAGIC_NUMBER (0xC8AD5DBC7DF4AD62)

//! Big scale test
BOOST_AUTO_TEST_CASE(test_big_scale)
{
	static const size_t testSize = 2000;

	DeviceCache deviceList[testSize];
	DeviceCache* devices[testSize];

	//! Fill database
	for (size_t index = 0; index < testSize; index++)
	{
		DeviceCache& deviceCache = deviceList[index];
		deviceCache.id = database->ReserveDevice();
		deviceCache.name = GenerateString(index % 256, index * 10);
		deviceCache.pluginID = MAGIC_NUMBER + index * 6;
		deviceCache.controllerID = MAGIC_NUMBER + index * 5;
		deviceCache.roomID = MAGIC_NUMBER + index * 4;

		//! Create device controller instance
		Ref<server::DeviceController> controller = deviceCache.controllerID ? MockDeviceController(deviceCache.controllerID) : nullptr;

		//! Create room instance
		Ref<server::Room> room = deviceCache.roomID ? MockRoom(deviceCache.roomID) : nullptr;

		Ref<server::Device> device = MockDevice(
			deviceCache.id,
			deviceCache.name,
			deviceCache.pluginID,
			controller,
			room);
		database->UpdateDevice(device);

		devices[index] = &deviceCache;
	}

	//! Verify database

	// Check table
	TestTableContent((const DeviceCache**)devices, testSize);

	// Check table size
	TestTableSize(testSize);
}