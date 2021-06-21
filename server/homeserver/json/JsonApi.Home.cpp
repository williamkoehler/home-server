#include "JsonApi.h"
#include "../Core.h"
#include "JsonField.h"

#include "../home/Home.h"
#include "../home/DeviceManager.h"
#include "../home/Room.h"
#include "../home/Device.h"
#include "../home/Action.h"

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

			boost::unordered::unordered_map<uint32_t, Ref<Device>>& deviceList = home->deviceList;

			// Reserve memory
			deviceListJson.Reserve(deviceList.size(), allocator);

			for (std::pair<uint32_t, Ref<Device>> item : deviceList)
			{
				rapidjson::Value deviceJson = rapidjson::Value(rapidjson::kObjectType);

				BuildJsonDevice(item.second, deviceJson, allocator);

				deviceListJson.PushBack(deviceJson, allocator);
			}

			output.AddMember("devices", deviceListJson, allocator);

			// Actions
			rapidjson::Value actionListJson = rapidjson::Value(rapidjson::kArrayType);

			boost::unordered::unordered_map<uint32_t, Ref<Action>>& actionList = home->actionList;

			// Reserve memory
			actionListJson.Reserve(actionList.size(), allocator);

			for (std::pair<uint32_t, Ref<Action>> item : actionList)
			{
				rapidjson::Value actionJson = rapidjson::Value(rapidjson::kObjectType);

				BuildJsonAction(item.second, actionJson, allocator);

				actionListJson.PushBack(actionJson, allocator);
			}

			output.AddMember("actions", actionListJson, allocator);

			// Rooms
			rapidjson::Value roomListJson = rapidjson::Value(rapidjson::kArrayType);

			boost::unordered::unordered_map<uint32_t, Ref<Room>>& roomList = home->roomList;

			// Reserve memory
			roomListJson.Reserve(roomList.size(), allocator);

			for (std::pair<uint32_t, Ref<Room>> item : roomList)
			{
				rapidjson::Value roomJson = rapidjson::Value(rapidjson::kObjectType);

				BuildJsonRoom(item.second, roomJson, allocator);

				roomListJson.PushBack(roomJson, allocator);
			}

			output.AddMember("rooms", roomListJson, allocator);

			// DeviceManagers
			rapidjson::Value deviceManagerListJson = rapidjson::Value(rapidjson::kArrayType);

			boost::unordered::unordered_map<uint32_t, Ref<DeviceManager>>& deviceManagerList = home->deviceManagerList;

			// Reserve memory
			deviceManagerListJson.Reserve(deviceManagerList.size(), allocator);

			for (std::pair<uint32_t, Ref<DeviceManager>> item : deviceManagerList)
			{
				rapidjson::Value deviceManagerJson = rapidjson::Value(rapidjson::kObjectType);

				BuildJsonDeviceManager(item.second, deviceManagerJson, allocator);

				deviceManagerListJson.PushBack(deviceManagerJson, allocator);
			}

			output.AddMember("devicemanagers", deviceManagerListJson, allocator);
		}
	}
	void JsonApi::DecodeJsonHome(rapidjson::Value& input)
	{
		assert(input.IsObject());

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Decode device managers if it exists
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

					Ref<Device> device = home->GetDevice_(deviceIDIt->value.GetUint());
					if (device != nullptr)
						DecodeJsonDevice(device, *deviceIt);
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

					Ref<Room> room = home->GetRoom_(roomIDIt->value.GetUint());
					if (room != nullptr)
						DecodeJsonRoom(room, *roomIt);
				}
			}
		}

		// Decode device managers if it exists
		rapidjson::Value::MemberIterator deviceManagerListIt = input.FindMember("devicemanagers");
		if (deviceManagerListIt != input.MemberEnd() && deviceManagerListIt->value.IsArray())
		{
			for (rapidjson::Value::ValueIterator deviceManagerIt = deviceManagerListIt->value.Begin(); deviceManagerIt != deviceManagerListIt->value.End(); deviceManagerIt++)
			{
				if (deviceManagerIt->IsObject())
				{
					rapidjson::Value::MemberIterator devicemanagerIDIt = input.FindMember("id");
					if (devicemanagerIDIt == input.MemberEnd() || !devicemanagerIDIt->value.IsUint())
						return;

					Ref<DeviceManager> deviceManager = home->GetDeviceManager_(devicemanagerIDIt->value.GetUint());
					if (deviceManager != nullptr)
						DecodeJsonDeviceManager(deviceManager, *deviceManagerIt);
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
		output.AddMember("type", rapidjson::Value(room->type), allocator);

		boost::container::set<WeakRef<Device>>& deviceList = room->deviceList;
		boost::container::set<WeakRef<Action>>& actionList = room->actionList;

		// Reserve memory
		rapidjson::Value devicesJson = rapidjson::Value(rapidjson::kArrayType);
		devicesJson.Reserve(deviceList.size(), allocator);

		for (WeakRef<Device>& deviceWeak : deviceList)
		{
			Ref<Device> device = deviceWeak.lock();
			if (device != nullptr)
				devicesJson.PushBack(rapidjson::Value(device->deviceID), allocator);
		}

		output.AddMember("devices", devicesJson, allocator);

		// Reserve memory
		rapidjson::Value actionsJson = rapidjson::Value(rapidjson::kArrayType);
		actionsJson.Reserve(actionList.size(), allocator);

		for (WeakRef<Action>& actionWeak : actionList)
		{
			Ref<Action> action = actionWeak.lock();
			if (action != nullptr)
				actionsJson.PushBack(rapidjson::Value(action->actionID), allocator);
		}

		output.AddMember("actions", actionsJson, allocator);
	}
	void JsonApi::DecodeJsonRoom(Ref<Room> room, rapidjson::Value& input)
	{
		assert(room != nullptr);
		assert(input.IsObject());

		// Decode room properties if they exist
		{
			boost::lock_guard lock(room->mutex);

			rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
			if (nameIt != input.MemberEnd() && nameIt->value.IsString())
				room->name.assign(nameIt->value.GetString(), nameIt->value.GetStringLength());

			rapidjson::Value::MemberIterator typeIt = input.FindMember("type");
			if (typeIt != input.MemberEnd() && typeIt->value.IsUint())
				room->type = typeIt->value.GetUint();
		}
	}

	void JsonApi::BuildJsonDevice(Ref<Device> device, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(device != nullptr);
		assert(output.IsObject());

		boost::shared_lock_guard lock(device->mutex);

		output.AddMember("name", rapidjson::Value(device->name.c_str(), device->name.size()), allocator);
		output.AddMember("id", rapidjson::Value(device->deviceID), allocator);
		output.AddMember("scriptid", rapidjson::Value(device->GetScriptID()), allocator);
	}
	void JsonApi::DecodeJsonDevice(Ref<Device> device, rapidjson::Value& input)
	{
		assert(device != nullptr);
		assert(input.IsObject());

		// Decode device properties if they exist
		{
			boost::lock_guard lock(device->mutex);

			rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
			if (nameIt != input.MemberEnd() && nameIt->value.IsString())
				device->name.assign(nameIt->value.GetString(), nameIt->value.GetStringLength());
		}
	}
	void JsonApi::BuildJsonDeviceState(Ref<Device> device, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(device != nullptr);
		assert(output.IsObject());

		// Lock mutex
		boost::shared_lock_guard lock(device->mutex);
		boost::lock_guard lock2(device->scriptMutex);

		// Check device manager script
		Ref<home::DeviceScript> script = device->script;
		if (script == nullptr)
		{
			LOG_ERROR("Invalid device script");
			return;
		}

		output.AddMember("id", rapidjson::Value(device->deviceID), allocator);

		rapidjson::Value fieldsJson = rapidjson::Value(rapidjson::kArrayType);
		JsonWriteableFieldCollection collection(fieldsJson, allocator);

		script->GetFields(collection);

		output.AddMember("fields", fieldsJson, allocator);
		output.AddMember("timestamp", rapidjson::Value(device->timestamp), allocator);
	}
	void JsonApi::DecodeJsonDeviceState(Ref<Device> device, rapidjson::Value& input, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(device != nullptr);
		assert(input.IsObject());

		// Lock mutex
		boost::lock_guard lock(device->scriptMutex);

		// Check device manager script
		Ref<home::DeviceScript> script = device->script;
		if (script == nullptr)
		{
			LOG_ERROR("Invalid device script");
			return;
		}

		// Decode device script properties if they exist
		{
			rapidjson::Value fieldsJson(rapidjson::kArrayType);
			JsonWriteableFieldCollection collection(fieldsJson, allocator);

			rapidjson::Value::MemberIterator fieldsIt = input.FindMember("fields");
			if (fieldsIt != input.MemberEnd() && fieldsIt->value.IsArray())
			{
				for (rapidjson::Value::ValueIterator it = fieldsIt->value.Begin(); it != fieldsIt->value.End(); it++)
				{
					if (it->IsObject())
					{
						rapidjson::Value::MemberIterator idIt = it->FindMember("id");
						rapidjson::Value::MemberIterator valueIt = it->FindMember("value");

						if (idIt != it->MemberEnd() && idIt->value.IsUint64() &&
							valueIt != it->MemberEnd())
						{
							JsonReadableField field(
								idIt->value.GetUint64(),
								valueIt->value);

							// Set field
							if (!script->SetField(field))
							{
								rapidjson::Value valueJson;
								JsonWriteableField field2(
									idIt->value.GetUint64(),
									valueJson,
									allocator);

								// If something went wrong get correct value from script
								if (script->GetField(field2))
									collection.AddField(field2);
							}
						}
					}
				}
			}

			output.AddMember("fields", fieldsJson, allocator);
		}
	}

	void JsonApi::BuildJsonDeviceManager(Ref<DeviceManager> deviceManager, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(deviceManager != nullptr);
		assert(output.IsObject());

		boost::shared_lock_guard lock(deviceManager->mutex);

		output.AddMember("name", rapidjson::Value(deviceManager->name.c_str(), deviceManager->name.size()), allocator);
		output.AddMember("id", rapidjson::Value(deviceManager->managerID), allocator);
		output.AddMember("scriptid", rapidjson::Value(deviceManager->GetScriptID()), allocator);
	}
	void JsonApi::DecodeJsonDeviceManager(Ref<DeviceManager> deviceManager, rapidjson::Value& input)
	{
		assert(deviceManager != nullptr);
		assert(input.IsObject());

		// Decode device manager properties if they exist
		{
			boost::lock_guard lock(deviceManager->mutex);

			rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
			if (nameIt != input.MemberEnd() && nameIt->value.IsString())
				deviceManager->name.assign(nameIt->value.GetString(), nameIt->value.GetStringLength());
		}
	}
	void JsonApi::BuildJsonDeviceManagerState(Ref<DeviceManager> deviceManager, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(deviceManager != nullptr);
		assert(output.IsObject());

		// Lock mutex
		boost::shared_lock_guard lock(deviceManager->mutex);
		boost::lock_guard lock2(deviceManager->scriptMutex);

		// Check device manager script
		Ref<home::DeviceManagerScript> script = deviceManager->script;
		if (script == nullptr)
		{
			LOG_ERROR("Invalid device manager script");
			return;
		}

		output.AddMember("id", rapidjson::Value(deviceManager->managerID), allocator);

		rapidjson::Value fieldListJson(rapidjson::kArrayType);
		JsonWriteableFieldCollection collection(fieldListJson, allocator);

		script->GetFields(collection);

		output.AddMember("fields", fieldListJson, allocator);
		output.AddMember("timestamp", rapidjson::Value(deviceManager->timestamp), allocator);
	}
	void JsonApi::DecodeJsonDeviceManagerState(Ref<DeviceManager> deviceManager, rapidjson::Value& input, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(deviceManager != nullptr);
		assert(input.IsObject());

		// Lock mutex
		boost::lock_guard lock(deviceManager->scriptMutex);

		// Check device manager script
		Ref<home::DeviceManagerScript> script = deviceManager->script;
		if (script == nullptr)
		{
			LOG_ERROR("Invalid device manager script");
			return;
		}

		// Decode device manager script properties if they exist
		{

			rapidjson::Value fieldsJson(rapidjson::kArrayType);
			JsonWriteableFieldCollection collection(fieldsJson, allocator);

			rapidjson::Value::MemberIterator fieldsIt = input.FindMember("fields");
			if (fieldsIt != input.MemberEnd() && fieldsIt->value.IsArray())
			{
				for (rapidjson::Value::ValueIterator it = fieldsIt->value.Begin(); it != fieldsIt->value.End(); it++)
				{
					if (it->IsObject())
					{
						rapidjson::Value::MemberIterator idIt = it->FindMember("id");
						rapidjson::Value::MemberIterator valueIt = it->FindMember("value");

						if (idIt != it->MemberEnd() && idIt->value.IsUint64() &&
							valueIt != it->MemberEnd())
						{
							JsonReadableField field(
								idIt->value.GetUint64(),
								valueIt->value);

							// Set field
							if (!script->SetField(field))
							{
								rapidjson::Value valueJson;
								JsonWriteableField field2(
									idIt->value.GetUint64(),
									valueJson,
									allocator);

								// If something went wrong get correct value from script
								if (script->GetField(field2))
									collection.AddField(field2);
							}
						}
					}
				}
			}

			output.AddMember("fields", fieldsJson, allocator);
		}
	}

	void JsonApi::BuildJsonAction(const Ref<Action>& action, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(action != nullptr);
		assert(output.IsObject());

		boost::shared_lock_guard lock(action->mutex);

		output.AddMember("name", rapidjson::Value(action->name.c_str(), action->name.size()), allocator);
		output.AddMember("id", rapidjson::Value(action->actionID), allocator);
		output.AddMember("sourceid", rapidjson::Value(action->GetDraftSourceID()), allocator);

		BuildJsonDraft(action->draft, output, allocator);
	}
	void JsonApi::DecodeJsonAction(const Ref<Action>& action, rapidjson::Value& input)
	{
		assert(action != nullptr);
		assert(input.IsObject());

		// Decode action properties if they exist
		{
			boost::lock_guard lock(action->mutex);

			rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
			if (nameIt != input.MemberEnd() && nameIt->value.IsString())
				action->name.assign(nameIt->value.GetString(), nameIt->value.GetStringLength());
		}
	}
	void JsonApi::BuildJsonActionState(const Ref<Action>& action, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(action != nullptr);
		assert(output.IsObject());

		// Lock mutex
		boost::shared_lock_guard lock(action->mutex);

		// Check device manager script
		Ref<scripting::Draft> draft = action->draft;
		if (draft == nullptr)
		{
			LOG_ERROR("Invalid action draft");
			return;
		}

		output.AddMember("id", rapidjson::Value(action->actionID), allocator);

		BuildJsonDraftState(draft, output, allocator);
	}
	void JsonApi::DecodeJsonActionState(const Ref<Action>& action, rapidjson::Value& input, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(action != nullptr);
		assert(input.IsObject());

		// Lock mutex
		boost::lock_guard lock(action->mutex);

		// Check device manager script
		Ref<scripting::Draft> draft = action->draft;
		if (draft == nullptr)
		{
			LOG_ERROR("Invalid action draft");
			return;
		}

		DecodeJsonDraftState(draft, input);
	}
}