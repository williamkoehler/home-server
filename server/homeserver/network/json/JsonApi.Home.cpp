#include "JsonApi.hpp"
#include "../../Core.hpp"

#include "../../home/Home.hpp"
#include "../../home/Room.hpp"
#include "../../home/DeviceController.hpp"
#include "../../home/Device.hpp"

namespace server
{
	// Home
	void JsonApi::BuildJsonHome(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator, size_t timestamp)
	{
		assert(output.IsObject());

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		boost::shared_lock_guard lock(home->mutex);

		output.AddMember("timestamp", rapidjson::Value(home->timestamp), allocator);

		// Only send tree if it has changed
		if (home->timestamp > timestamp)
		{
			// Devices
			rapidjson::Value deviceListJson = rapidjson::Value(rapidjson::kArrayType);

			boost::unordered::unordered_map<identifier_t, Ref<Device>>& deviceList = home->deviceList;

			// Reserve memory
			deviceListJson.Reserve(deviceList.size(), allocator);

			for (std::pair<identifier_t, Ref<Device>> item : deviceList)
			{
				rapidjson::Value deviceJson = rapidjson::Value(rapidjson::kObjectType);

				BuildJsonDevice(item.second, deviceJson, allocator);

				deviceListJson.PushBack(deviceJson, allocator);
			}

			output.AddMember("devices", deviceListJson, allocator);

			// DeviceControllers
			rapidjson::Value deviceControllerListJson = rapidjson::Value(rapidjson::kArrayType);

			boost::unordered::unordered_map<identifier_t, Ref<DeviceController>>& deviceControllerList = home->deviceControllerList;

			// Reserve memory
			deviceControllerListJson.Reserve(deviceControllerList.size(), allocator);

			for (std::pair<identifier_t, Ref<DeviceController>> item : deviceControllerList)
			{
				rapidjson::Value deviceControllerJson = rapidjson::Value(rapidjson::kObjectType);

				BuildJsonDeviceController(item.second, deviceControllerJson, allocator);

				deviceControllerListJson.PushBack(deviceControllerJson, allocator);
			}

			output.AddMember("devicecontrollers", deviceControllerListJson, allocator);

			// Rooms
			rapidjson::Value roomListJson = rapidjson::Value(rapidjson::kArrayType);

			boost::unordered::unordered_map<identifier_t, Ref<Room>>& roomList = home->roomList;

			// Reserve memory
			roomListJson.Reserve(roomList.size(), allocator);

			for (std::pair<identifier_t, Ref<Room>> item : roomList)
			{
				rapidjson::Value roomJson = rapidjson::Value(rapidjson::kObjectType);

				BuildJsonRoom(item.second, roomJson, allocator);

				roomListJson.PushBack(roomJson, allocator);
			}

			output.AddMember("rooms", roomListJson, allocator);
		}
	}
	void JsonApi::DecodeJsonHome(rapidjson::Value& input)
	{
		assert(input.IsObject());

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Decode devices if it exists
		rapidjson::Value::MemberIterator deviceListIt = input.FindMember("devices");
		if (deviceListIt != input.MemberEnd() && deviceListIt->value.IsArray())
		{
			for (rapidjson::Value::ValueIterator deviceIt = deviceListIt->value.Begin(); deviceIt != deviceListIt->value.End(); deviceIt++)
			{
				if (deviceIt->IsObject())
				{
					rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
					if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
						return;

					Ref<Device> device = home->GetDevice(deviceIDIt->value.GetUint64());
					if (device != nullptr)
						DecodeJsonDevice(device, *deviceIt);
				}
			}
		}

		// Decode devicecontrollers if it exists
		rapidjson::Value::MemberIterator deviceControllerListIt = input.FindMember("devicecontrollers");
		if (deviceControllerListIt != input.MemberEnd() && deviceControllerListIt->value.IsArray())
		{
			for (rapidjson::Value::ValueIterator deviceControllerIt = deviceControllerListIt->value.Begin(); deviceControllerIt != deviceControllerListIt->value.End(); deviceControllerIt++)
			{
				if (deviceControllerIt->IsObject())
				{
					rapidjson::Value::MemberIterator controllerIDIt = input.FindMember("id");
					if (controllerIDIt == input.MemberEnd() || !controllerIDIt->value.IsUint())
						return;

					Ref<DeviceController> controller = home->GetDeviceController(controllerIDIt->value.GetUint64());
					if (controller != nullptr)
						DecodeJsonDeviceController(controller, *deviceControllerIt);
				}
			}
		}

		// Decode rooms if it exists
		rapidjson::Value::MemberIterator roomListIt = input.FindMember("rooms");
		if (roomListIt != input.MemberEnd() && roomListIt->value.IsArray())
		{
			for (rapidjson::Value::ValueIterator roomIt = roomListIt->value.Begin(); roomIt != roomListIt->value.End(); roomIt++)
			{
				if (roomIt->IsObject())
				{
					rapidjson::Value::MemberIterator roomIDIt = input.FindMember("id");
					if (roomIDIt == input.MemberEnd() || !roomIDIt->value.IsUint())
						return;

					Ref<Room> room = home->GetRoom(roomIDIt->value.GetUint64());
					if (room != nullptr)
						DecodeJsonRoom(room, *roomIt);
				}
			}
		}
	}

	void JsonApi::BuildJsonRoom(Ref<Room> room, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(room != nullptr);
		assert(output.IsObject());

		boost::shared_lock_guard lock(room->mutex);

		output.AddMember("name", rapidjson::Value(room->name.c_str(), room->name.size()), allocator);
		output.AddMember("id", rapidjson::Value(room->roomID), allocator);
		output.AddMember("type", rapidjson::Value(room->type.c_str(), room->type.size()), allocator);

		// Build devices
		boost::container::set<identifier_t>& deviceList = room->deviceList;

		// Reserve memory
		rapidjson::Value devicesJson = rapidjson::Value(rapidjson::kArrayType);
		devicesJson.Reserve(deviceList.size(), allocator);

		for (identifier_t& deviceID : deviceList)
			devicesJson.PushBack(rapidjson::Value(deviceID), allocator);

		output.AddMember("devices", devicesJson, allocator);

		// Build devicecontrollers
		boost::container::set<identifier_t>& deviceControllerList = room->deviceControllerList;

		// Reserve memory
		rapidjson::Value devicecontrollersJson = rapidjson::Value(rapidjson::kArrayType);
		devicecontrollersJson.Reserve(deviceList.size(), allocator);

		for (identifier_t& controllerID : deviceControllerList)
			devicecontrollersJson.PushBack(rapidjson::Value(controllerID), allocator);

		output.AddMember("devicecontrollers", devicecontrollersJson, allocator);
	}
	void JsonApi::DecodeJsonRoom(Ref<Room> room, rapidjson::Value& input)
	{
		assert(room != nullptr);
		assert(input.IsObject());

		// Decode properties
		rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
		if (nameIt != input.MemberEnd() && nameIt->value.IsString())
			room->SetName(std::string(nameIt->value.GetString(), nameIt->value.GetStringLength()));

		rapidjson::Value::MemberIterator typeIt = input.FindMember("type");
		if (typeIt != input.MemberEnd() && typeIt->value.IsUint())
			room->SetType(std::string(typeIt->value.GetString(), typeIt->value.GetStringLength()));
	}

	void JsonApi::BuildJsonDeviceController(Ref<DeviceController> controller, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(controller != nullptr);
		assert(output.IsObject());

		// Lock
		boost::shared_lock_guard lock(controller->mutex);

		// Build properties
		output.AddMember("name", rapidjson::Value(controller->name.c_str(), controller->name.size()), allocator);
		output.AddMember("id", rapidjson::Value(controller->controllerID), allocator);
		output.AddMember("pluginid", rapidjson::Value(controller->GetPluginID()), allocator);
		output.AddMember("roomid",
						 controller->room != nullptr ?
						 rapidjson::Value(controller->room->GetRoomID()) : rapidjson::Value(rapidjson::kNullType), allocator);
	}
	void JsonApi::DecodeJsonDeviceController(Ref<DeviceController> controller, rapidjson::Value& input)
	{
		assert(controller != nullptr);
		assert(input.IsObject());

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Decode properties
		rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
		if (nameIt != input.MemberEnd() && nameIt->value.IsString())
			controller->SetName(std::string(nameIt->value.GetString(), nameIt->value.GetStringLength()));

		rapidjson::Value::MemberIterator roomIDIt = input.FindMember("roomid");
		if (roomIDIt != input.MemberEnd() && roomIDIt->value.IsUint64())
			controller->SetRoom(home->GetRoom(roomIDIt->value.GetUint64()));
	}
	void JsonApi::BuildJsonDeviceControllerState(Ref<DeviceController> controller, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(controller != nullptr);
		assert(output.IsObject());

		// Lock mutex
		boost::shared_lock_guard lock(controller->mutex);

		// Build properties
		output.AddMember("id", rapidjson::Value(controller->controllerID), allocator);
		output.AddMember("state", rapidjson::Value(controller->snapshot, allocator), allocator);
	}
	void JsonApi::DecodeJsonDeviceControllerState(Ref<DeviceController> controller, rapidjson::Value& input, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(controller != nullptr);
		assert(input.IsObject());

		//// Lock mutex
		//boost::lock_guard lock(device->scriptMutex);

		//// Check device manager script
		//Ref<home::DeviceScript> script = device->script;
		//if (script == nullptr)
		//{
		//	LOG_ERROR("Invalid device script");
		//	return;
		//}

		//// Decode device script properties if they exist
		//{
		//	rapidjson::Value fieldsJson(rapidjson::kArrayType);
		//	JsonWriteableFieldCollection collection(fieldsJson, allocator);

		//	rapidjson::Value::MemberIterator fieldsIt = input.FindMember("fields");
		//	if (fieldsIt != input.MemberEnd() && fieldsIt->value.IsArray())
		//	{
		//		for (rapidjson::Value::ValueIterator it = fieldsIt->value.Begin(); it != fieldsIt->value.End(); it++)
		//		{
		//			if (it->IsObject())
		//			{
		//				rapidjson::Value::MemberIterator idIt = it->FindMember("id");
		//				rapidjson::Value::MemberIterator valueIt = it->FindMember("value");

		//				if (idIt != it->MemberEnd() && idIt->value.IsUint64() &&
		//					valueIt != it->MemberEnd())
		//				{
		//					JsonReadableField field(
		//						idIt->value.GetUint64(),
		//						valueIt->value);

		//					// Set field
		//					if (!script->SetField(field))
		//					{
		//						rapidjson::Value valueJson;
		//						JsonWriteableField field2(
		//							idIt->value.GetUint64(),
		//							valueJson,
		//							allocator);

		//						// If something went wrong get correct value from script
		//						if (script->GetField(field2))
		//							collection.AddField(field2);
		//					}
		//				}
		//			}
		//		}
		//	}

		//	output.AddMember("fields", fieldsJson, allocator);
		//}
	}

	void JsonApi::BuildJsonDevice(Ref<Device> device, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(device != nullptr);
		assert(output.IsObject());

		// Lock
		boost::shared_lock_guard lock(device->mutex);

		// Build properties
		output.AddMember("name", rapidjson::Value(device->name.c_str(), device->name.size()), allocator);
		output.AddMember("id", rapidjson::Value(device->deviceID), allocator);
		output.AddMember("pluginid", rapidjson::Value(device->GetPluginID()), allocator);
		output.AddMember("roomid",
						 device->room != nullptr ?
						 rapidjson::Value(device->room->GetRoomID()) : rapidjson::Value(rapidjson::kNullType), allocator);
	}
	void JsonApi::DecodeJsonDevice(Ref<Device> device, rapidjson::Value& input)
	{
		assert(device != nullptr);
		assert(input.IsObject());

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Decode properties
		rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
		if (nameIt != input.MemberEnd() && nameIt->value.IsString())
			device->SetName(std::string(nameIt->value.GetString(), nameIt->value.GetStringLength()));

		rapidjson::Value::MemberIterator roomIDIt = input.FindMember("roomid");
		if (roomIDIt != input.MemberEnd() && roomIDIt->value.IsUint64())
			device->SetRoom(home->GetRoom(roomIDIt->value.GetUint64()));
	}
	void JsonApi::BuildJsonDeviceState(Ref<Device> device, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(device != nullptr);
		assert(output.IsObject());

		// Lock
		boost::shared_lock_guard lock(device->mutex);

		// Build properties
		output.AddMember("id", rapidjson::Value(device->deviceID), allocator);
		output.AddMember("state", rapidjson::Value(device->snapshot, allocator), allocator);
	}
	void JsonApi::DecodeJsonDeviceState(Ref<Device> device, rapidjson::Value& input, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(device != nullptr);
		assert(input.IsObject());

		//// Lock mutex
		//boost::lock_guard lock(device->scriptMutex);

		//// Check device manager script
		//Ref<home::DeviceScript> script = device->script;
		//if (script == nullptr)
		//{
		//	LOG_ERROR("Invalid device script");
		//	return;
		//}

		//// Decode device script properties if they exist
		//{
		//	rapidjson::Value fieldsJson(rapidjson::kArrayType);
		//	JsonWriteableFieldCollection collection(fieldsJson, allocator);

		//	rapidjson::Value::MemberIterator fieldsIt = input.FindMember("fields");
		//	if (fieldsIt != input.MemberEnd() && fieldsIt->value.IsArray())
		//	{
		//		for (rapidjson::Value::ValueIterator it = fieldsIt->value.Begin(); it != fieldsIt->value.End(); it++)
		//		{
		//			if (it->IsObject())
		//			{
		//				rapidjson::Value::MemberIterator idIt = it->FindMember("id");
		//				rapidjson::Value::MemberIterator valueIt = it->FindMember("value");

		//				if (idIt != it->MemberEnd() && idIt->value.IsUint64() &&
		//					valueIt != it->MemberEnd())
		//				{
		//					JsonReadableField field(
		//						idIt->value.GetUint64(),
		//						valueIt->value);

		//					// Set field
		//					if (!script->SetField(field))
		//					{
		//						rapidjson::Value valueJson;
		//						JsonWriteableField field2(
		//							idIt->value.GetUint64(),
		//							valueJson,
		//							allocator);

		//						// If something went wrong get correct value from script
		//						if (script->GetField(field2))
		//							collection.AddField(field2);
		//					}
		//				}
		//			}
		//		}
		//	}

		//	output.AddMember("fields", fieldsJson, allocator);
		//}
	}
}