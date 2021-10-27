#include "HomeModule.hpp"
#include "Script.hpp"
#include "../../home/Home.hpp"
#include "../../home/Room.hpp"
#include "../../home/Device.hpp"
#include "../../home/DeviceController.hpp"

namespace server
{
	namespace javascript
	{
		#define HOME_OBJECT DUK_HIDDEN_SYMBOL("Home")
		#define HOME_POINTER DUK_HIDDEN_SYMBOL("home_ptr")

		#define DEVICE_OBJECT DUK_HIDDEN_SYMBOL("Device")
		#define DEVICE_POINTER DUK_HIDDEN_SYMBOL("device_ptr")

		#define ROOM_OBJECT DUK_HIDDEN_SYMBOL("Room")
		#define ROOM_POINTER DUK_HIDDEN_SYMBOL("room_ptr")

		bool HomeModule::Import(duk_context* context)
		{
			assert(context != nullptr);

			// Import home
			{
				duk_push_c_function(context, Home::Constructor, 1);
				duk_push_object(context); // Prototype

				// Register methods
				static const duk_function_list_entry homeMethods[] = {
					{ "getAction", Home::GetDevice, 1 },
					{ nullptr, nullptr, 0 }
				};

				duk_put_function_list(context, -1, homeMethods);

				// Register prototype and action
				duk_put_prop_string(context, -2, "prototype");
				duk_put_global_string(context, HOME_OBJECT);
			}

			// Import device
			{
				duk_push_c_function(context, Device::Constructor, 1);
				duk_push_object(context); // Prototype

				// Register methods
				static const duk_function_list_entry actionMethods[] = {
					{ "getName", Device::GetDeviceName, 0 },
					{ "setName", Device::SetDeviceName, 1 },
					{ nullptr, nullptr, 0 }
				};

				duk_put_function_list(context, -1, actionMethods);

				// Register prototype and action
				duk_put_prop_string(context, -2, "prototype");
				duk_put_global_string(context, DEVICE_OBJECT);
			}

			return true;
		}

		// Home
		duk_ret_t Home::Constructor(duk_context* context)
		{
			if (!duk_is_constructor_call(context))
				return DUK_RET_ERROR;

			// Expect []
			if (duk_get_top(context) != 0)
				return DUK_RET_ERROR;

			// Get this [this]
			duk_push_this(context);

			Ref<server::Home> home = server::Home::GetInstance();
			assert(home != nullptr);

			// Set pointer
			duk_push_pointer(context, home.get());
			duk_put_prop_string(context, -2, HOME_POINTER);

			return 0;
		}

		duk_ret_t Home::GetDevice(duk_context* context)
		{
			// Expect [uint]
			if (duk_get_top(context) != 0)
				return DUK_RET_ERROR;

			// Get id
			uint32_t id = duk_get_uint(context, -1);
			if (id == 0)
				return DUK_RET_ERROR;

			// Get this
			duk_push_this(context); // [number this]

			// Get pointer
			duk_get_prop_string(context, -1, HOME_POINTER); // [number this pointer]
			server::Home* home = (server::Home*)duk_get_pointer(context, -1);
			if (home == nullptr)
				return DUK_RET_ERROR;

			// Pop 3
			duk_pop_3(context); /// []

			// Get action
			Ref<server::Device> device = home->GetDevice(id);
			if (device == nullptr)
				return DUK_RET_ERROR;

			// New action object
			Device::New(context, device);

			return 1;
		}

		bool Home::New(duk_context* context, Ref<server::Home> home)
		{
			assert(context != nullptr);
			assert(home != nullptr);

			// New action object
			duk_get_global_string(context, HOME_OBJECT); // [Home]
			duk_push_pointer(context, home.get()); // [Home pointer]
			duk_new(context, 1); // [object]

			return true;
		}

		// Device
		duk_ret_t Device::Constructor(duk_context* context)
		{
			if (!duk_is_constructor_call(context))
				return DUK_RET_ERROR;

			// Expect [pointer]
			if (duk_get_top(context) != 1)
				return DUK_RET_ERROR;

			// Get this
			duk_push_this(context); // [pointer this]

			// Set pointer
			duk_dup(context, 0); // [pointer this pointer]
			duk_put_prop_string(context, -2, DEVICE_POINTER); // [pointer this]

			return 0;
		}

		duk_ret_t Device::GetDeviceName(duk_context* context)
		{
			// Expect []
			if (duk_get_top(context) != 0)
				return DUK_RET_ERROR;

			// Get this
			duk_push_this(context); // [this]

			// Get pointer
			duk_get_prop_string(context, -1, DEVICE_POINTER); // [this pointer]
			server::Device* device = (server::Device*)duk_get_pointer(context, -1);
			if (device == nullptr)
				return DUK_RET_ERROR;

			// Pop 2
			duk_pop_2(context); // []

			// Get name
			std::string name = device->GetName();
			duk_push_lstring(context, name.data(), name.size()); // [string]

			return 1;
		}
		duk_ret_t Device::SetDeviceName(duk_context* context)
		{
			// Expect [string]
			if (duk_get_top(context) != 1)
				return DUK_RET_ERROR;

			// Get value
			size_t valueLength;
			const char* value = duk_get_lstring(context, -1, &valueLength);

			// Get this
			duk_push_this(context); // [string this]

			// Get pointer
			duk_get_prop_string(context, -1, DEVICE_POINTER); // [string this pointer]
			server::Device* device = (server::Device*)duk_get_pointer(context, -1);
			if (device == nullptr)
				return DUK_RET_ERROR;

			// Pop 3
			duk_pop_3(context); // []

			// Set name
			device->SetName(std::string(value, valueLength));

			return 1;
		}

		bool Device::New(duk_context* context, Ref<server::Device> device)
		{
			assert(context != nullptr);
			assert(device != nullptr);

			// New action object
			duk_get_global_string(context, DEVICE_OBJECT); // [Action]
			duk_push_pointer(context, device.get()); // [Action pointer]
			duk_new(context, 1); // [object]

			return true;
		}
	}
}