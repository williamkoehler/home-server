#include "JsonApi.h"
#include "../Core.h"

#include "../home/Home.h"
#include "../home/DeviceManager.h"
#include "../home/Room.h"
#include "../home/Device.h"
#include "../home/Action.h"

#include "../user/UserManager.h"
#include "../user/User.h"

namespace server
{
	// Home
	void JsonApi::ProcessJsonGetHomeMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
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
	void JsonApi::ProcessJsonSetHomeMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		DecodeJsonHome(input);

		BuildJsonAckMessageWS(output);
	}

	// Room
	void JsonApi::ProcessJsonAddRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
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
			typeIt == input.MemberEnd() || !typeIt->value.IsUint())
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
		Ref<Room> room = home->AddRoom(nameIt->value.GetString(), 0, typeIt->value.GetUint(), json);
		if (room == nullptr)
		{
			context.Error("Add room");
			BuildJsonNAckMessageWS(output);
			return;
		}

		BuildJsonRoom(room, output, allocator);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonRemoveRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
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

	void JsonApi::ProcessJsonGetRoomMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
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
		Ref<Room> room = home->GetRoom_(roomIDIt->value.GetUint());
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
	void JsonApi::ProcessJsonSetRoomMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
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
		Ref<Room> room = home->GetRoom_(roomIDIt->value.GetUint());
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

	void JsonApi::ProcessJsonAddDeviceToRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
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
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("deviceid");
		if (roomIDIt == input.MemberEnd() || !roomIDIt->value.IsUint() ||
			deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
		{
			context.Error("Missing id and/or deviceid");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get room
		Ref<Room> room = home->GetRoom_(roomIDIt->value.GetUint());
		if (room == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Get room
		Ref<Device> device = home->GetDevice_(deviceIDIt->value.GetUint());
		if (device == nullptr)
		{
			context.Error("Invalid deviceid");
			BuildJsonNAckMessageWS(output);
			return;
		}

		room->AddDevice(device);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonRemoveDeviceFromRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
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
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("deviceid");
		if (roomIDIt == input.MemberEnd() || !roomIDIt->value.IsUint() ||
			deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
		{
			context.Error("Missing id and/or deviceid");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get room
		Ref<Room> room = home->GetRoom_(roomIDIt->value.GetUint());
		if (room == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Get room
		Ref<Device> device = home->GetDevice_(deviceIDIt->value.GetUint());
		if (device == nullptr)
		{
			context.Error("Invalid deviceid");
			BuildJsonNAckMessageWS(output);
			return;
		}

		room->RemoveDevice(device);

		BuildJsonAckMessageWS(output);
	}

	// DeviceManager
	void JsonApi::ProcessJsonAddDeviceManagerMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
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
		rapidjson::Value::MemberIterator scriptIDIt = input.FindMember("scriptid");
		if (nameIt == input.MemberEnd() || !nameIt->value.IsString() ||
			scriptIDIt == input.MemberEnd() || !scriptIDIt->value.IsUint())
		{
			context.Error("Missing name and/or scriptid");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);
		if (home->AddDeviceManager(nameIt->value.GetString(), 0, scriptIDIt->value.GetUint(), json) == nullptr)
		{
			context.Error("Add device manager");
			BuildJsonNAckMessageWS(output);
			return;
		}

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonRemoveDeviceManagerMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
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
		rapidjson::Value::MemberIterator deviceManagerIDIt = input.FindMember("id");
		if (deviceManagerIDIt == input.MemberEnd() || !deviceManagerIDIt->value.IsUint())
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
			home->RemoveDeviceManager(deviceManagerIDIt->value.GetUint());
		}
		catch (std::exception)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		BuildJsonAckMessageWS(output);
	}

	void JsonApi::ProcessJsonGetDeviceManagerMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceManagerIDIt = input.FindMember("id");
		if (deviceManagerIDIt == input.MemberEnd() || !deviceManagerIDIt->value.IsUint())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get device manager
		Ref<DeviceManager> deviceManager = home->GetDeviceManager_(deviceManagerIDIt->value.GetUint());
		if (deviceManager == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build device manager
		BuildJsonDeviceManager(deviceManager, output, allocator);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonSetDeviceManagerMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceManagerIDIt = input.FindMember("id");
		if (deviceManagerIDIt == input.MemberEnd() || !deviceManagerIDIt->value.IsUint())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get device manager
		Ref<DeviceManager> deviceManager = home->GetDeviceManager_(deviceManagerIDIt->value.GetUint());
		if (deviceManager == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Decode device manager
		DecodeJsonDeviceManager(deviceManager, input);

		BuildJsonAckMessageWS(output);
	}

	void JsonApi::ProcessJsonGetDeviceManagerStateMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceManagerIDIt = input.FindMember("id");
		if (deviceManagerIDIt == input.MemberEnd() || !deviceManagerIDIt->value.IsUint())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get device manager
		Ref<DeviceManager> deviceManager = home->GetDeviceManager_(deviceManagerIDIt->value.GetUint());
		if (deviceManager == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build device manager state
		BuildJsonDeviceManagerState(deviceManager, output, allocator);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonSetDeviceManagerStateMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceManagerIDIt = input.FindMember("id");
		if (deviceManagerIDIt == input.MemberEnd() || !deviceManagerIDIt->value.IsUint())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Get device manager
		Ref<DeviceManager> deviceManager = home->GetDeviceManager_(deviceManagerIDIt->value.GetUint());
		if (deviceManager == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		output.AddMember("id", deviceManagerIDIt->value, allocator);

		// Decode device manager state
		DecodeJsonDeviceManagerState(deviceManager, input, output, allocator);

		BuildJsonAckMessageWS(output);
	}

	void JsonApi::ProcessJsonGetDeviceManagerStatesMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		boost::shared_lock_guard lock(home->mutex);

		rapidjson::Value deviceManagerListJson = rapidjson::Value(rapidjson::kArrayType);

		boost::unordered::unordered_map<uint32_t, Ref<DeviceManager>>& deviceManagerList = home->deviceManagerList;
		for (std::pair<uint32_t, Ref<DeviceManager>> item : deviceManagerList)
		{
			assert(item.second != nullptr);

			rapidjson::Value deviceManagerJson = rapidjson::Value(rapidjson::kObjectType);

			BuildJsonDeviceManagerState(item.second, deviceManagerJson, allocator);

			deviceManagerListJson.PushBack(deviceManagerJson, allocator);
		}

		output.AddMember("devicemanagers", deviceManagerListJson, allocator);

		BuildJsonAckMessageWS(output);
	}

	// Device
	void JsonApi::ProcessJsonAddDeviceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
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
		rapidjson::Value::MemberIterator scriptIDIt = input.FindMember("scriptid");
		if (nameIt == input.MemberEnd() || !nameIt->value.IsString() ||
			scriptIDIt == input.MemberEnd() || !scriptIDIt->value.IsUint())
		{
			context.Error("Missing name and/or scriptid");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);
		if (home->AddDevice(nameIt->value.GetString(), 0, scriptIDIt->value.GetUint(), json) == nullptr)
		{
			context.Error("Add device");
			BuildJsonNAckMessageWS(output);
			return;
		}

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonRemoveDeviceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
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
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
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
			home->RemoveDevice(deviceIDIt->value.GetUint());
		}
		catch (std::exception)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		BuildJsonAckMessageWS(output);
	}

	void JsonApi::ProcessJsonGetDeviceMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
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
		Ref<Device> device = home->GetDevice_(deviceIDIt->value.GetUint());
		if (device == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build device
		BuildJsonDevice(device, output, allocator);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonSetDeviceMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
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
		Ref<Device> device = home->GetDevice_(deviceIDIt->value.GetUint());
		if (device == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Decode device
		DecodeJsonDevice(device, input);

		BuildJsonAckMessageWS(output);
	}

	void JsonApi::ProcessJsonGetDeviceStateMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
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
		Ref<Device> device = home->GetDevice_(deviceIDIt->value.GetUint());
		if (device == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build device state
		BuildJsonDeviceState(device, output, allocator);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonSetDeviceStateMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
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
		Ref<Device> device = home->GetDevice_(deviceIDIt->value.GetUint());
		if (device == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		output.AddMember("id", deviceIDIt->value, allocator);

		// Decode device state
		DecodeJsonDeviceState(device, input, output, allocator);

		BuildJsonAckMessageWS(output);
	}

	void JsonApi::ProcessJsonGetDeviceStatesMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();

		assert(home != nullptr);

		boost::shared_lock_guard lock(home->mutex);

		rapidjson::Value deviceListJson = rapidjson::Value(rapidjson::kArrayType);

		boost::unordered::unordered_map<uint32_t, Ref<Device>>& deviceList = home->deviceList;
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

	// Action
	void JsonApi::ProcessJsonAddActionMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
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
		rapidjson::Value::MemberIterator sourceIDIt = input.FindMember("sourceid");
		if (nameIt == input.MemberEnd() || !nameIt->value.IsString() ||
			sourceIDIt == input.MemberEnd() || !sourceIDIt->value.IsUint())
		{
			context.Error("Missing name and/or sourceid");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);
		if (home->AddAction(nameIt->value.GetString(), 0, sourceIDIt->value.GetUint(), json) == nullptr)
		{
			context.Error("Add action");
			BuildJsonNAckMessageWS(output);
			return;
		}

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonRemoveActionMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
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
		rapidjson::Value::MemberIterator actionIDIt = input.FindMember("id");
		if (actionIDIt == input.MemberEnd() || !actionIDIt->value.IsUint())
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
			home->RemoveAction(actionIDIt->value.GetUint());
		}
		catch (std::exception)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		BuildJsonAckMessageWS(output);
	}

	void JsonApi::ProcessJsonGetActionMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator actionIDIt = input.FindMember("id");
		if (actionIDIt == input.MemberEnd() || !actionIDIt->value.IsUint())
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
		Ref<Action> action = home->GetAction_(actionIDIt->value.GetUint());
		if (action == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build device
		BuildJsonAction(action, output, allocator);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonSetActionMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator actionIDIt = input.FindMember("id");
		if (actionIDIt == input.MemberEnd() || !actionIDIt->value.IsUint())
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
		Ref<Action> action = home->GetAction_(actionIDIt->value.GetUint());
		if (action == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Decode action
		DecodeJsonAction(action, input);

		BuildJsonAckMessageWS(output);
	}

	void JsonApi::ProcessJsonGetActionStateMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator actionIDIt = input.FindMember("id");
		if (actionIDIt == input.MemberEnd() || !actionIDIt->value.IsUint())
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
		Ref<Action> action = home->GetAction_(actionIDIt->value.GetUint());
		if (action == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build device state
		BuildJsonActionState(action, output, allocator);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonSetActionStateMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
		if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
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
		Ref<Action> action = home->GetAction_(deviceIDIt->value.GetUint());
		if (action == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		output.AddMember("id", deviceIDIt->value, allocator);

		// Decode device state
		DecodeJsonActionState(action, input, output, allocator);

		BuildJsonAckMessageWS(output);
	}

	void JsonApi::ProcessJsonResetActionMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{ }
	void JsonApi::ProcessJsonExecuteActionMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator actionIDIt = input.FindMember("id");
		if (actionIDIt == input.MemberEnd() || !actionIDIt->value.IsUint())
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
		Ref<Action> action = home->GetAction_(actionIDIt->value.GetUint());
		if (action == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		scripting::ExecutionResult result = action->Execute();

		// Handle errors
		if (result.IsSuccess())
			BuildJsonAckMessageWS(output);
		else
		{
			context.Error("Action execution : %s", result.GetError().c_str());
			BuildJsonNAckMessageWS(output);
		}
	}
}