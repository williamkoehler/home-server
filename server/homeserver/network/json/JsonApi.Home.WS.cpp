#include "JsonApi.hpp"
#include "../../Core.hpp"

#include "../../home/Home.hpp"
#include "../../home/Room.hpp"
#include "../../home/DeviceController.hpp"
#include "../../home/Device.hpp"

#include "../../user/UserManager.hpp"
#include "../../user/User.hpp"

namespace server
{
	// Home
	void JsonApi::ProcessJsonGetHomeMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator timestampIt = input.FindMember("timestamp");

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		size_t timestamp = (timestampIt != input.MemberEnd() && timestampIt->value.IsUint64()) ? timestampIt->value.GetUint64() : 0;

		BuildJsonHome(output, allocator, timestamp);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonSetHomeMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		DecodeJsonHome(input);

		BuildJsonAckMessageWS(output);
	}

	// Room
	void JsonApi::ProcessJsonAddRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel != UserAccessLevels::kAdministratorUserAccessLevel)
		{
			context.Error("Invalid access level. User needs to be administrator.");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
		rapidjson::Value::MemberIterator typeIt = input.FindMember("type");
		if (nameIt == input.MemberEnd() || !nameIt->value.IsString() ||
			typeIt == input.MemberEnd() || !typeIt->value.IsString())
		{
			context.Error("Missing name and/or type");
			BuildJsonNAckMessageWS(output);
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
			context.Error("Add room");
			BuildJsonNAckMessageWS(output);
			return;
		}

		BuildJsonRoom(room, output, allocator);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonRemoveRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel != UserAccessLevels::kAdministratorUserAccessLevel)
		{
			context.Error("Invalid access level. User needs to be administrator.");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator roomIDIt = input.FindMember("id");
		if (roomIDIt == input.MemberEnd() || !roomIDIt->value.IsUint())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		try
		{
			home->RemoveRoom(roomIDIt->value.GetUint());
		}

		catch (std::exception)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		BuildJsonAckMessageWS(output);
	}

	void JsonApi::ProcessJsonGetRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator roomIDIt = input.FindMember("id");
		if (roomIDIt == input.MemberEnd() || !roomIDIt->value.IsUint())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
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
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build room
		BuildJsonRoom(room, output, allocator);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonSetRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator roomIDIt = input.FindMember("id");
		if (roomIDIt == input.MemberEnd() || !roomIDIt->value.IsUint())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
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
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Decode room
		DecodeJsonRoom(room, input);

		BuildJsonAckMessageWS(output);
	}

	// DeviceController
	void JsonApi::ProcessJsonAddDeviceControllerMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel != UserAccessLevels::kAdministratorUserAccessLevel)
		{
			context.Error("Invalid access level. User needs to be administrator.");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
		rapidjson::Value::MemberIterator pluginIDIt = input.FindMember("pluginid");
		rapidjson::Value::MemberIterator roomIDIt = input.FindMember("roomid");
		if (nameIt == input.MemberEnd() || !nameIt->value.IsString() ||
			pluginIDIt == input.MemberEnd() || !pluginIDIt->value.IsUint64() ||
			roomIDIt == input.MemberEnd() || (!roomIDIt->value.IsUint64() && !roomIDIt->value.IsNull()))
		{
			context.Error("Missing name, pluginid and/or roomid");
			BuildJsonNAckMessageWS(output);
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
			pluginIDIt->value.GetUint64(),
			roomIDIt->value.IsUint64() ? roomIDIt->value.GetUint64() : 0,
			json);
		if (controller == nullptr)
		{
			context.Error("Add controller");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		BuildJsonDeviceController(controller, output, allocator);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonRemoveDeviceControllerMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel != UserAccessLevels::kAdministratorUserAccessLevel)
		{
			context.Error("Invalid access level. User needs to be administrator.");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator controllerIDIt = input.FindMember("id");
		if (controllerIDIt == input.MemberEnd() || !controllerIDIt->value.IsUint64())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Remove device
		if (!home->RemoveDeviceController(controllerIDIt->value.GetUint64()))
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
		}
		else
			BuildJsonAckMessageWS(output);
	}

	void JsonApi::ProcessJsonGetDeviceControllerMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator controllerIDIt = input.FindMember("id");
		if (controllerIDIt == input.MemberEnd() || !controllerIDIt->value.IsUint64())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();

		assert(home != nullptr);

		// Get device controller
		Ref<DeviceController> controller = home->GetDeviceController(controllerIDIt->value.GetUint64());
		if (controller == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build properties
		BuildJsonDeviceController(controller, output, allocator);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonSetDeviceControllerMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint64())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get device
		Ref<DeviceController> controller = home->GetDeviceController(deviceIDIt->value.GetUint64());
		if (controller == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Decode properties
		DecodeJsonDeviceController(controller, input);

		BuildJsonAckMessageWS(output);
	}

	void JsonApi::ProcessJsonGetDeviceControllerStateMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint64())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();

		assert(home != nullptr);

		// Get device
		Ref<DeviceController> controller = home->GetDeviceController(deviceIDIt->value.GetUint64());
		if (controller == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build properties
		BuildJsonDeviceControllerState(controller, output, allocator);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonSetDeviceControllerStateMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint64())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get device
		Ref<DeviceController> controller = home->GetDeviceController(deviceIDIt->value.GetUint64());
		if (controller == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		output.AddMember("id", deviceIDIt->value, allocator);

		// Decode properties
		DecodeJsonDeviceControllerState(controller, input, output, allocator);

		BuildJsonAckMessageWS(output);
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
		boost::unordered::unordered_map<identifier_t, Ref<DeviceController>>& deviceControllerList = home->deviceControllerList;
		for (std::pair<uint32_t, Ref<DeviceController>> item : deviceControllerList)
		{
			assert(item.second != nullptr);

			rapidjson::Value deviceControllerJson = rapidjson::Value(rapidjson::kObjectType);

			BuildJsonDeviceControllerState(item.second, deviceControllerJson, allocator);

			deviceControllerListJson.PushBack(deviceControllerJson, allocator);
		}

		output.AddMember("devicecontrollers", deviceControllerListJson, allocator);

		BuildJsonAckMessageWS(output);
	}

	// Device
	void JsonApi::ProcessJsonAddDeviceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel != UserAccessLevels::kAdministratorUserAccessLevel)
		{
			context.Error("Invalid access level. User needs to be administrator.");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
		rapidjson::Value::MemberIterator pluginIDIt = input.FindMember("pluginid");
		rapidjson::Value::MemberIterator controllerIDIt = input.FindMember("controllerid");
		rapidjson::Value::MemberIterator roomIDIt = input.FindMember("roomid");
		if (nameIt == input.MemberEnd() || !nameIt->value.IsString() ||
			pluginIDIt == input.MemberEnd() || !pluginIDIt->value.IsUint64() ||
			controllerIDIt == input.MemberEnd() || (!controllerIDIt->value.IsUint64() && !controllerIDIt->value.IsNull()) ||
			roomIDIt == input.MemberEnd() || (!roomIDIt->value.IsUint64() && !roomIDIt->value.IsNull()))
		{
			context.Error("Missing name, pluginid, controllerid, and/or roomid");
			BuildJsonNAckMessageWS(output);
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
			pluginIDIt->value.GetUint64(),
			controllerIDIt->value.IsUint64() ? controllerIDIt->value.GetUint64() : 0,
			roomIDIt->value.IsUint64() ? roomIDIt->value.GetUint64() : 0,
			json);
		if (device == nullptr)
		{
			context.Error("Add device");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		BuildJsonDevice(device, output, allocator);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonRemoveDeviceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel != UserAccessLevels::kAdministratorUserAccessLevel)
		{
			context.Error("Invalid access level. User needs to be administrator.");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint64())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Remove device
		if (!home->RemoveDevice(deviceIDIt->value.GetUint64()))
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
		}
		else
			BuildJsonAckMessageWS(output);
	}

	void JsonApi::ProcessJsonGetDeviceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint64())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get device
		Ref<Device> device = home->GetDevice(deviceIDIt->value.GetUint64());
		if (device == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build properties
		BuildJsonDevice(device, output, allocator);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonSetDeviceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint64())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get device
		Ref<Device> device = home->GetDevice(deviceIDIt->value.GetUint64());
		if (device == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Decode properties
		DecodeJsonDevice(device, input);

		BuildJsonAckMessageWS(output);
	}

	void JsonApi::ProcessJsonGetDeviceStateMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint64())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get device
		Ref<Device> device = home->GetDevice(deviceIDIt->value.GetUint64());
		if (device == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build properties
		BuildJsonDeviceState(device, output, allocator);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonSetDeviceStateMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint64())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get device
		Ref<Device> device = home->GetDevice(deviceIDIt->value.GetUint64());
		if (device == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		output.AddMember("id", deviceIDIt->value, allocator);

		// Decode properties
		DecodeJsonDeviceState(device, input, output, allocator);

		BuildJsonAckMessageWS(output);
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
		boost::unordered::unordered_map<identifier_t, Ref<Device>>& deviceList = home->deviceList;
		for (std::pair<uint32_t, Ref<Device>> item : deviceList)
		{
			assert(item.second != nullptr);

			rapidjson::Value deviceJson = rapidjson::Value(rapidjson::kObjectType);

			BuildJsonDeviceState(item.second, deviceJson, allocator);

			deviceListJson.PushBack(deviceJson, allocator);
		}

		output.AddMember("devices", deviceListJson, allocator);

		BuildJsonAckMessageWS(output);
	}
}