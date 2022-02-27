#include "JsonApi.hpp"
#include "../../home/Home.hpp"
#include "../../home/Room.hpp"
#include "../../home/DeviceController.hpp"
#include "../../home/Device.hpp"
#include "../../home/Action.hpp"

#include "../../user/UserManager.hpp"
#include "../../user/User.hpp"

namespace server
{
	//! Home
	void JsonApi::ProcessJsonGetHomeMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator timestampIt = input.FindMember("timestamp");

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		size_t timestamp = (timestampIt != input.MemberEnd() && timestampIt->value.IsUint()) ? timestampIt->value.GetUint() : 0;

		BuildJsonHome(output, allocator, timestamp);
	}
	void JsonApi::ProcessJsonSetHomeMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		DecodeJsonHome(input);
	}

	//! Room
	void JsonApi::ProcessJsonAddRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel < UserAccessLevel::kMaintainerUserAccessLevel)
		{
			context.Error(ApiError::kError_AccessLevelToLow);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
		rapidjson::Value::MemberIterator typeIt = input.FindMember("type");
		if (nameIt == input.MemberEnd() || !nameIt->value.IsString() ||
			typeIt == input.MemberEnd() || !typeIt->value.IsString())
		{
			context.Error("Missing name and/or type");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);
		Ref<Room> room = home->AddRoom(
			std::string(nameIt->value.GetString(), nameIt->value.GetStringLength()),
			std::string(typeIt->value.GetString(), typeIt->value.GetStringLength()),
			json);
		if (room == nullptr)
		{
			//! Error failed to add room
			context.Error(ApiError::kError_InternalError);
			return;
		}

		BuildJsonRoom(room, output, allocator);
	}
	void JsonApi::ProcessJsonRemoveRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel < UserAccessLevel::kMaintainerUserAccessLevel)
		{
			context.Error(ApiError::kError_AccessLevelToLow);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator roomIDIt = input.FindMember("id");
		if (roomIDIt == input.MemberEnd() || !roomIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		if (!home->RemoveRoom(roomIDIt->value.GetUint()))
		{
			//! Error failed to remove room
			context.Error(ApiError::kError_InternalError);
			return;
		}
	}

	void JsonApi::ProcessJsonGetRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator roomIDIt = input.FindMember("id");
		if (roomIDIt == input.MemberEnd() || !roomIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get room
		Ref<Room> room = home->GetRoom(roomIDIt->value.GetUint());
		if (room == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Build room
		BuildJsonRoom(room, output, allocator);
	}
	void JsonApi::ProcessJsonSetRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator roomIDIt = input.FindMember("id");
		if (roomIDIt == input.MemberEnd() || !roomIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();

		assert(home != nullptr);

		// Get device
		Ref<Room> room = home->GetRoom(roomIDIt->value.GetUint());
		if (room == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Decode room
		DecodeJsonRoom(room, input);
	}

	//! DeviceController
	void JsonApi::ProcessJsonAddDeviceControllerMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel < UserAccessLevel::kMaintainerUserAccessLevel)
		{
			context.Error(ApiError::kError_AccessLevelToLow);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
		rapidjson::Value::MemberIterator pluginIDIt = input.FindMember("pluginid");
		rapidjson::Value::MemberIterator roomIDIt = input.FindMember("roomid");
		if (nameIt == input.MemberEnd() || !nameIt->value.IsString() ||
			pluginIDIt == input.MemberEnd() || !pluginIDIt->value.IsUint() ||
			roomIDIt == input.MemberEnd() || (!roomIDIt->value.IsUint() && !roomIDIt->value.IsNull()))
		{
			context.Error("Missing name, pluginid and/or roomid");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Add new device controller
		rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);
		Ref<DeviceController> controller = home->AddDeviceController(
			nameIt->value.GetString(),
			pluginIDIt->value.GetUint(),
			roomIDIt->value.IsUint() ? roomIDIt->value.GetUint() : 0,
			json);
		if (controller == nullptr)
		{
			//! Error failed to add device controller
			context.Error(ApiError::kError_InternalError);
			return;
		}

		// Build response
		BuildJsonDeviceController(controller, output, allocator);
	}
	void JsonApi::ProcessJsonRemoveDeviceControllerMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel < UserAccessLevel::kMaintainerUserAccessLevel)
		{
			context.Error(ApiError::kError_AccessLevelToLow);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator controllerIDIt = input.FindMember("id");
		if (controllerIDIt == input.MemberEnd() || !controllerIDIt->value.IsUint())
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Remove device
		if (!home->RemoveDeviceController(controllerIDIt->value.GetUint()))
		{
			//! Error failed to remove device controller
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}
	}

	void JsonApi::ProcessJsonGetDeviceControllerMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator controllerIDIt = input.FindMember("id");
		if (controllerIDIt == input.MemberEnd() || !controllerIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();

		assert(home != nullptr);

		// Get device controller
		Ref<DeviceController> controller = home->GetDeviceController(controllerIDIt->value.GetUint());
		if (controller == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Build properties
		BuildJsonDeviceController(controller, output, allocator);
	}
	void JsonApi::ProcessJsonSetDeviceControllerMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get device
		Ref<DeviceController> controller = home->GetDeviceController(deviceIDIt->value.GetUint());
		if (controller == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Decode properties
		DecodeJsonDeviceController(controller, input);
	}

	void JsonApi::ProcessJsonInvokeDeviceControllerEventMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		rapidjson::Value::MemberIterator eventIt = input.FindMember("event");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint() ||
			eventIt == input.MemberEnd() || !eventIt->value.IsString())
		{
			context.Error("Missing id and/or event");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();

		assert(home != nullptr);

		// Get device
		Ref<DeviceController> controller = home->GetDeviceController(deviceIDIt->value.GetUint());
		if (controller == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Invoke event
		controller->Invoke(std::string(eventIt->value.GetString(), eventIt->value.GetStringLength()));
	}

	void JsonApi::ProcessJsonGetDeviceControllerStateMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();

		assert(home != nullptr);

		// Get device
		Ref<DeviceController> controller = home->GetDeviceController(deviceIDIt->value.GetUint());
		if (controller == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Build properties
		BuildJsonDeviceControllerState(controller, output, allocator);
	}
	void JsonApi::ProcessJsonSetDeviceControllerStateMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get device
		Ref<DeviceController> controller = home->GetDeviceController(deviceIDIt->value.GetUint());
		if (controller == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		output.AddMember("id", deviceIDIt->value, allocator);

		// Decode properties
		DecodeJsonDeviceControllerState(controller, input, output, allocator);
	}

	void JsonApi::ProcessJsonGetDeviceControllerStatesMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		boost::shared_lock_guard lock(home->mutex);

		rapidjson::Value deviceControllerListJson = rapidjson::Value(rapidjson::kArrayType);

		// Parse every device controller
		for (auto [id, controller] : home->deviceControllerList)
		{
			assert(controller != nullptr);

			rapidjson::Value deviceControllerJson = rapidjson::Value(rapidjson::kObjectType);

			BuildJsonDeviceControllerState(controller, deviceControllerJson, allocator);

			deviceControllerListJson.PushBack(deviceControllerJson, allocator);
		}

		output.AddMember("devicecontrollers", deviceControllerListJson, allocator);
	}

	//! Device
	void JsonApi::ProcessJsonAddDeviceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel < UserAccessLevel::kMaintainerUserAccessLevel)
		{
			context.Error(ApiError::kError_AccessLevelToLow);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
		rapidjson::Value::MemberIterator pluginIDIt = input.FindMember("pluginid");
		rapidjson::Value::MemberIterator controllerIDIt = input.FindMember("controllerid");
		rapidjson::Value::MemberIterator roomIDIt = input.FindMember("roomid");
		if (nameIt == input.MemberEnd() || !nameIt->value.IsString() ||
			pluginIDIt == input.MemberEnd() || !pluginIDIt->value.IsUint() ||
			controllerIDIt == input.MemberEnd() || (!controllerIDIt->value.IsUint() && !controllerIDIt->value.IsNull()) ||
			roomIDIt == input.MemberEnd() || (!roomIDIt->value.IsUint() && !roomIDIt->value.IsNull()))
		{
			context.Error("Missing name, pluginid, controllerid, and/or roomid");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Add new device
		rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);
		Ref<Device> device = home->AddDevice(
			nameIt->value.GetString(),
			pluginIDIt->value.GetUint(),
			controllerIDIt->value.IsUint() ? controllerIDIt->value.GetUint() : 0,
			roomIDIt->value.IsUint() ? roomIDIt->value.GetUint() : 0,
			json);
		if (device == nullptr)
		{
			//! Error failed to add device
			context.Error(ApiError::kError_InternalError);
			return;
		}

		// Build response
		BuildJsonDevice(device, output, allocator);
	}
	void JsonApi::ProcessJsonRemoveDeviceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel < UserAccessLevel::kMaintainerUserAccessLevel)
		{
			context.Error(ApiError::kError_AccessLevelToLow);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Remove device
		if (!home->RemoveDevice(deviceIDIt->value.GetUint()))
		{
			//! Error failed to remove device
			context.Error(ApiError::kError_InternalError);
			return;
		}
	}

	void JsonApi::ProcessJsonGetDeviceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get device
		Ref<Device> device = home->GetDevice(deviceIDIt->value.GetUint());
		if (device == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Build properties
		BuildJsonDevice(device, output, allocator);
	}
	void JsonApi::ProcessJsonSetDeviceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get device
		Ref<Device> device = home->GetDevice(deviceIDIt->value.GetUint());
		if (device == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Decode properties
		DecodeJsonDevice(device, input);
	}

	void JsonApi::ProcessJsonInvokeDeviceEventMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		rapidjson::Value::MemberIterator eventIt = input.FindMember("event");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint() ||
			eventIt == input.MemberEnd() || !eventIt->value.IsString())
		{
			context.Error("Missing id and/or event");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get device
		Ref<Device> device = home->GetDevice(deviceIDIt->value.GetUint());
		if (device == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Invoke event
		device->Invoke(std::string(eventIt->value.GetString(), eventIt->value.GetStringLength()));
	}

	void JsonApi::ProcessJsonGetDeviceStateMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get device
		Ref<Device> device = home->GetDevice(deviceIDIt->value.GetUint());
		if (device == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Build properties
		BuildJsonDeviceState(device, output, allocator);
	}
	void JsonApi::ProcessJsonSetDeviceStateMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get device
		Ref<Device> device = home->GetDevice(deviceIDIt->value.GetUint());
		if (device == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		output.AddMember("id", deviceIDIt->value, allocator);

		// Decode properties
		DecodeJsonDeviceState(device, input, output, allocator);
	}

	void JsonApi::ProcessJsonGetDeviceStatesMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		boost::shared_lock_guard lock(home->mutex);

		rapidjson::Value deviceListJson = rapidjson::Value(rapidjson::kArrayType);

		// Parse every device
		for (auto [id, device] : home->deviceList)
		{
			assert(device != nullptr);

			rapidjson::Value deviceJson = rapidjson::Value(rapidjson::kObjectType);

			BuildJsonDeviceState(device, deviceJson, allocator);

			deviceListJson.PushBack(deviceJson, allocator);
		}

		output.AddMember("devices", deviceListJson, allocator);
	}

	//! Action
	void JsonApi::ProcessJsonAddActionMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel < UserAccessLevel::kMaintainerUserAccessLevel)
		{
			context.Error(ApiError::kError_AccessLevelToLow);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
		rapidjson::Value::MemberIterator sourceIDIt = input.FindMember("sourceid");
		rapidjson::Value::MemberIterator roomIDIt = input.FindMember("roomid");
		if (nameIt == input.MemberEnd() || !nameIt->value.IsString() ||
			sourceIDIt == input.MemberEnd() || !sourceIDIt->value.IsUint() ||
			roomIDIt == input.MemberEnd() || (!roomIDIt->value.IsUint() && !roomIDIt->value.IsNull()))
		{
			context.Error("Missing name, sourceid, and/or roomid");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Add new device
		rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);
		Ref<Action> action = home->AddAction(
			nameIt->value.GetString(),
			sourceIDIt->value.GetUint(),
			roomIDIt->value.IsUint() ? roomIDIt->value.GetUint() : 0,
			json);
		if (action == nullptr)
		{
			//! Error failed to add action
			context.Error(ApiError::kError_InternalError);
			return;
		}

		// Build response
		BuildJsonAction(action, output, allocator);
	}
	void JsonApi::ProcessJsonRemoveActionMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel < UserAccessLevel::kMaintainerUserAccessLevel)
		{
			context.Error(ApiError::kError_AccessLevelToLow);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator actionIDIt = input.FindMember("id");
		if (actionIDIt == input.MemberEnd() || !actionIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Remove action
		if (!home->RemoveAction(actionIDIt->value.GetUint()))
		{
			//! Error failed to remove action
			context.Error(ApiError::kError_InternalError);
			return;
		}
	}

	void JsonApi::ProcessJsonGetActionMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator actionIDIt = input.FindMember("id");
		if (actionIDIt == input.MemberEnd() || !actionIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get action
		Ref<Action> action = home->GetAction(actionIDIt->value.GetUint());
		if (action == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Build properties
		BuildJsonAction(action, output, allocator);
	}
	void JsonApi::ProcessJsonSetActionMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator actionIDIt = input.FindMember("id");
		if (actionIDIt == input.MemberEnd() || !actionIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get action
		Ref<Action> action = home->GetAction(actionIDIt->value.GetUint());
		if (action == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Decode properties
		DecodeJsonAction(action, input);
	}

	void JsonApi::ProcessJsonInvokeActionEventMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator actionIDIt = input.FindMember("id");
		rapidjson::Value::MemberIterator eventIt = input.FindMember("event");
		if (actionIDIt == input.MemberEnd() || !actionIDIt->value.IsUint() ||
			eventIt == input.MemberEnd() || !eventIt->value.IsString())
		{
			context.Error("Missing id and/or event");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get action
		Ref<Action> action = home->GetAction(actionIDIt->value.GetUint());
		if (action == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Invoke event
		action->Invoke(std::string(eventIt->value.GetString(), eventIt->value.GetStringLength()));
	}

	void JsonApi::ProcessJsonGetActionStateMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator actionIDIt = input.FindMember("id");
		if (actionIDIt == input.MemberEnd() || !actionIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get device
		Ref<Action> action = home->GetAction(actionIDIt->value.GetUint());
		if (action == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Build properties
		BuildJsonActionState(action, output, allocator);
	}
	void JsonApi::ProcessJsonSetActionStateMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator actionIDIt = input.FindMember("id");
		if (actionIDIt == input.MemberEnd() || !actionIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get action
		Ref<Action> action = home->GetAction(actionIDIt->value.GetUint());
		if (action == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		output.AddMember("id", actionIDIt->value, allocator);

		// Decode properties
		DecodeJsonActionState(action, input, output, allocator);
	}

	void JsonApi::ProcessJsonGetActionStatesMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		boost::shared_lock_guard lock(home->mutex);

		rapidjson::Value actionListJson = rapidjson::Value(rapidjson::kArrayType);

		// Parse every action
		for (auto [id, action] : home->actionList)
		{
			assert(action != nullptr);

			rapidjson::Value actionJson = rapidjson::Value(rapidjson::kObjectType);

			BuildJsonActionState(action, actionJson, allocator);

			actionListJson.PushBack(actionJson, allocator);
		}

		output.AddMember("actions", actionListJson, allocator);
	}
}