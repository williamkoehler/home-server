#include "JSHomeModule.hpp"
#include "JSScript.hpp"
#include "JSConstants.hpp"
#include "../../home/Home.hpp"
#include "../../home/Room.hpp"
#include "../../home/Device.hpp"
#include "../../home/DeviceController.hpp"

namespace server
{
	namespace javascript
	{
		bool JSHomeModule::Import(duk_context* context)
		{
			assert(context != nullptr);

			// Import room
			{
				duk_push_c_function(context, JSRoom::Constructor, 1);
				duk_push_object(context); // Prototype

				// Register methods
				static const duk_function_list_entry methods[] = {
					{ "isValid", JSRoom::IsValid, 0 },
					{ "getName", JSRoom::GetName, 0 },
					{ "setName", JSRoom::SetName, 1 },
					{ nullptr, nullptr, 0 }
				};

				duk_put_function_list(context, -1, methods);

				// Register prototype and constructor
				duk_put_prop_string(context, -2, "prototype");
				duk_put_global_string(context, ROOM_OBJECT);
			}

			// Import device
			{
				duk_push_c_function(context, JSDevice::Constructor, 1);
				duk_push_object(context); // Prototype

				// Register methods
				static const duk_function_list_entry methods[] = {
					{ "isValid", JSDevice::IsValid, 0 },
					{ "getName", JSDevice::GetName, 0 },
					{ "setName", JSDevice::SetName, 1 },
					{ nullptr, nullptr, 0 }
				};

				duk_put_function_list(context, -1, methods);

				// Register prototype and constructor
				duk_put_prop_string(context, -2, "prototype");
				duk_put_global_string(context, DEVICE_OBJECT);
			}

			// Import device controller
			{
				duk_push_c_function(context, JSDeviceController::Constructor, 1);
				duk_push_object(context); // Prototype

				// Register methods
				static const duk_function_list_entry methods[] = {
					{ "isValid", JSDeviceController::IsValid, 0 },
					{ "getName", JSDeviceController::GetName, 0 },
					{ "setName", JSDeviceController::SetName, 1 },
					{ nullptr, nullptr, 0 }
				};

				duk_put_function_list(context, -1, methods);

				// Register prototype and constructor
				duk_put_prop_string(context, -2, "prototype");
				duk_put_global_string(context, DEVICECONTROLLER_OBJECT);
			}

			return true;
		}

		//! Room
		duk_ret_t JSRoom::Constructor(duk_context* context)
		{
			if (!duk_is_constructor_call(context))
				return DUK_RET_ERROR;

			// Expect [integer]
			if (duk_get_top(context) != 1 || !duk_is_number(context, -1))
				return DUK_RET_ERROR;

			// Get this
			duk_push_this(context); // [number this]

			// Set integer
			duk_dup(context, 0); // [number this number]
			duk_put_prop_string(context, -2, UNIQUE_ID); // [number this]

			return 0;
		}

		duk_ret_t JSRoom::IsValid(duk_context* context)
		{
			// Expect []
			if (duk_get_top(context) != 0)
				return DUK_RET_ERROR;

			// Get id
			duk_push_this(context); // [this]
			duk_get_prop_string(context, -1, UNIQUE_ID); // [this number]
			identifier_t roomID = (identifier_t)duk_get_uint(context, -1);

			// Pop 2
			duk_pop_2(context); // []

			// Get room
			Ref<Home> home = Home::GetInstance();
			assert(home != nullptr);

			Ref<Room> room = home->GetRoom(roomID);
			duk_push_boolean(context, room != nullptr);

			return 1;
		}

		duk_ret_t JSRoom::GetName(duk_context* context)
		{
			// Expect []
			if (duk_get_top(context) != 0)
				return DUK_RET_ERROR;

			// Get id
			duk_push_this(context); // [this]
			duk_get_prop_string(context, -1, UNIQUE_ID); // [this number]
			identifier_t roomID = (identifier_t)duk_get_uint(context, -1);

			// Get room
			Ref<Home> home = Home::GetInstance();
			assert(home != nullptr);

			Ref<Room> room = home->GetRoom(roomID);
			if (room == nullptr)
				return DUK_RET_ERROR;

			// Pop 2
			duk_pop_2(context); // []

			// Get name
			std::string name = room->GetName();
			duk_push_lstring(context, name.data(), name.size()); // [string]

			return 1;
		}
		duk_ret_t JSRoom::SetName(duk_context* context)
		{
			// Expect [string]
			if (duk_get_top(context) != 1 || !duk_is_string(context, -1))
				return DUK_RET_ERROR;

			// Get value
			size_t valueLength;
			const char* value = duk_get_lstring(context, -1, &valueLength);

			// Get id
			duk_push_this(context); // [string this]
			duk_get_prop_string(context, -1, UNIQUE_ID); // [string this number]
			identifier_t roomID = (identifier_t)duk_get_uint(context, -1);

			// Get room
			Ref<Home> home = Home::GetInstance();
			assert(home != nullptr);

			Ref<Room> room = home->GetRoom(roomID);
			if (room == nullptr)
				return DUK_RET_ERROR;

			// Set name
			room->SetName(std::string(value, valueLength));

			// Pop 3
			duk_pop_3(context); // []

			return 0;
		}

		bool JSRoom::New(duk_context* context, Ref<Room> room)
		{
			assert(context != nullptr);
			assert(room != nullptr);

			// New room object
			duk_get_global_string(context, ROOM_OBJECT); // [function]
			duk_push_uint(context, room->GetRoomID()); // [function number]
			duk_new(context, 1); // [object]

			return true;
		}

		//! Device
		duk_ret_t JSDevice::Constructor(duk_context* context)
		{
			if (!duk_is_constructor_call(context))
				return DUK_RET_ERROR;

			// Expect [integer]
			if (duk_get_top(context) != 1 || !duk_is_number(context, -1))
				return DUK_RET_ERROR;

			// Get this
			duk_push_this(context); // [number this]

			// Set integer
			duk_dup(context, 0); // [number this number]
			duk_put_prop_string(context, -2, UNIQUE_ID); // [number this]

			return 0;
		}

		duk_ret_t JSDevice::IsValid(duk_context* context)
		{
			// Expect []
			if (duk_get_top(context) != 0)
				return DUK_RET_ERROR;

			// Get id
			duk_push_this(context); // [this]
			duk_get_prop_string(context, -1, UNIQUE_ID); // [this number]
			identifier_t deviceID = (identifier_t)duk_get_uint(context, -1);

			// Pop 2
			duk_pop_2(context); // []

			// Get device
			Ref<Home> home = Home::GetInstance();
			assert(home != nullptr);

			Ref<Device> device = home->GetDevice(deviceID);
			duk_push_boolean(context, device != nullptr);

			return 1;
		}

		duk_ret_t JSDevice::GetName(duk_context* context)
		{
			// Expect []
			if (duk_get_top(context) != 0)
				return DUK_RET_ERROR;

			// Get id
			duk_push_this(context); // [this]
			duk_get_prop_string(context, -1, UNIQUE_ID); // [this number]
			identifier_t deviceID = (identifier_t)duk_get_uint(context, -1);

			// Get device
			Ref<Home> home = Home::GetInstance();
			assert(home != nullptr);

			Ref<Device> device = home->GetDevice(deviceID);
			if (device == nullptr)
				return DUK_RET_ERROR;

			// Pop 2
			duk_pop_2(context); // []

			// Get name
			std::string name = device->GetName();
			duk_push_lstring(context, name.data(), name.size()); // [string]

			return 1;
		}
		duk_ret_t JSDevice::SetName(duk_context* context)
		{
			// Expect [string]
			if (duk_get_top(context) != 1 || !duk_is_string(context, -1))
				return DUK_RET_ERROR;

			// Get value
			size_t valueLength;
			const char* value = duk_get_lstring(context, -1, &valueLength);

			// Get id
			duk_push_this(context); // [string this]
			duk_get_prop_string(context, -1, UNIQUE_ID); // [string this number]
			identifier_t deviceID = (identifier_t)duk_get_uint(context, -1);

			// Get device
			Ref<Home> home = Home::GetInstance();
			assert(home != nullptr);

			Ref<Device> device = home->GetDevice(deviceID);
			if (device == nullptr)
				return DUK_RET_ERROR;

			// Set name
			device->SetName(std::string(value, valueLength));

			// Pop 3
			duk_pop_3(context); // []

			return 0;
		}

		bool JSDevice::New(duk_context* context, Ref<Device> device)
		{
			assert(context != nullptr);
			assert(device != nullptr);

			// New device object
			duk_get_global_string(context, DEVICE_OBJECT); // [function]
			duk_push_uint(context, device->GetDeviceID()); // [function number]
			duk_new(context, 1); // [object]

			return true;
		}

		//! DeviceController
		duk_ret_t JSDeviceController::Constructor(duk_context* context)
		{
			if (!duk_is_constructor_call(context))
				return DUK_RET_ERROR;

			// Expect [integer]
			if (duk_get_top(context) != 1 || !duk_is_number(context, -1))
				return DUK_RET_ERROR;

			// Get this
			duk_push_this(context); // [number this]

			// Set integer
			duk_dup(context, 0); // [number this number]
			duk_put_prop_string(context, -2, UNIQUE_ID); // [number this]

			return 0;
		}

		duk_ret_t JSDeviceController::IsValid(duk_context* context)
		{
			// Expect []
			if (duk_get_top(context) != 0)
				return DUK_RET_ERROR;

			// Get id
			duk_push_this(context); // [this]
			duk_get_prop_string(context, -1, UNIQUE_ID); // [this number]
			identifier_t controllerID = (identifier_t)duk_get_uint(context, -1);

			// Pop 2
			duk_pop_2(context); // []

			// Get device
			Ref<Home> home = Home::GetInstance();
			assert(home != nullptr);

			Ref<DeviceController> controller = home->GetDeviceController(controllerID);
			duk_push_boolean(context, controller != nullptr);

			return 1;
		}

		duk_ret_t JSDeviceController::GetName(duk_context* context)
		{
			// Expect []
			if (duk_get_top(context) != 0)
				return DUK_RET_ERROR;

			// Get id
			duk_push_this(context); // [this]
			duk_get_prop_string(context, -1, UNIQUE_ID); // [this number]
			identifier_t controllerID = (identifier_t)duk_get_uint(context, -1);

			// Get device
			Ref<Home> home = Home::GetInstance();
			assert(home != nullptr);

			Ref<DeviceController> controller = home->GetDeviceController(controllerID);
			if (controller == nullptr)
				return DUK_RET_ERROR;

			// Pop 2
			duk_pop_2(context); // []

			// Get name
			std::string name = controller->GetName();
			duk_push_lstring(context, name.data(), name.size()); // [string]

			return 1;
		}
		duk_ret_t JSDeviceController::SetName(duk_context* context)
		{
			// Expect [string]
			if (duk_get_top(context) != 1 || !duk_is_string(context, -1))
				return DUK_RET_ERROR;

			// Get value
			size_t valueLength;
			const char* value = duk_get_lstring(context, -1, &valueLength);

			// Get id
			duk_push_this(context); // [string this]
			duk_get_prop_string(context, -1, UNIQUE_ID); // [string this number]
			identifier_t controllerID = (identifier_t)duk_get_uint(context, -1);

			// Get device
			Ref<Home> home = Home::GetInstance();
			assert(home != nullptr);

			Ref<DeviceController> controller = home->GetDeviceController(controllerID);
			if (controller == nullptr)
				return DUK_RET_ERROR;

			// Set name
			controller->SetName(std::string(value, valueLength));

			// Pop 3
			duk_pop_3(context); // []

			return 0;
		}

		bool JSDeviceController::New(duk_context* context, Ref<DeviceController> controller)
		{
			assert(context != nullptr);
			assert(controller != nullptr);

			// New device controller object
			duk_get_global_string(context, DEVICECONTROLLER_OBJECT); // [function]
			duk_push_uint(context, controller->GetDeviceControllerID()); // [function number]
			duk_new(context, 1); // [object]

			return true;
		}
	}
}