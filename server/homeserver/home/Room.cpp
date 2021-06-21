#include "Room.h"
#include "../Core.h"
#include "Home.h"
#include "Device.h"
#include "DeviceManager.h"
#include "Action.h"
#include "../plugin/PluginManager.h"
#include <xxHash/xxhash.h>

namespace server
{
	Room::Room(std::string name, uint32_t roomID, uint32_t type)
		: name(std::move(name)), roomID(roomID), type(type)
	{ 	}
	Room::~Room()
	{
		// Decrement all room counts of all remaining devices
		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		for (WeakRef<Device>& deviceWeak : deviceList)
		{
			Ref<Device> device = deviceWeak.lock();
			if (device != nullptr)
			{
				boost::lock_guard lock(device->mutex);
				device->roomCount--;
			}
		}
		deviceList.clear();

		for (WeakRef<Action>& actionWeak : actionList)
		{
			Ref<Action> action = actionWeak.lock();
			if (action != nullptr)
			{
				boost::lock_guard lock(action->mutex);
				action->roomCount--;
			}
		}
		actionList.clear();
	}
	Ref<Room> Room::Create(std::string name, uint32_t roomID, uint32_t type)
	{
		Ref<Room> room = boost::make_shared<Room>(std::move(name), roomID, type);
		if (room == nullptr)
			return nullptr;

		return room;
	}

	void Room::AddDevice(Ref<Device> device)
	{
		assert(device != nullptr);

		boost::lock_guard lock(mutex);

		// Add device id to list
		if (deviceList.insert(device).second)
		{
			// Increment room count
			device->roomCount++;
		}
	}
	void Room::RemoveDevice(Ref<Device> device)
	{
		assert(device != nullptr);

		boost::lock_guard lock(mutex);

		// Remove device id from list
		if (deviceList.erase(device))
		{
			// Decrement room count
			device->roomCount--;
		}
	}

	void Room::AddAction(Ref<Action> action)
	{
		assert(action != nullptr);

		boost::lock_guard lock(mutex);

		// Add device id to list
		if (actionList.insert(action).second)
		{
			// Increment room count
			action->roomCount++;
		}
	}

	void Room::RemoveAction(Ref<Action> action)
	{
		assert(action != nullptr);

		boost::lock_guard lock(mutex);

		// Remove device id from list
		if (actionList.erase(action))
		{
			// Decrement room count
			action->roomCount--;
		}
	}

	//IO
	void Room::Load(rapidjson::Value& json)
	{
		assert(json.IsObject());

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		//Read device list
		rapidjson::Value::MemberIterator deviceListIt = json.FindMember("device-list");
		if (deviceListIt != json.MemberEnd() && deviceListIt->value.IsArray())
		{
			rapidjson::Value& deviceListJson = deviceListIt->value;
			for (rapidjson::Value::ValueIterator deviceIt = deviceListJson.Begin(); deviceIt != deviceListJson.End(); deviceIt++)
			{
				if (!deviceIt->IsUint())
				{
					LOG_ERROR("Missing device id in 'device-list' in room");
					throw std::runtime_error("Invalid file 'home-info.json'");
				}

				Ref<Device> device = home->GetDevice_(deviceIt->GetUint());
				if (device == nullptr)
				{
					LOG_WARNING("Invalid device id in 'device-list' in room");
					continue;
				}

				AddDevice(device);
			}
		}

		//Read action list
		rapidjson::Value::MemberIterator actionListIt = json.FindMember("action-list");
		if (actionListIt != json.MemberEnd() && actionListIt->value.IsArray())
		{
			rapidjson::Value& actionListJson = actionListIt->value;
			for (rapidjson::Value::ValueIterator actionIt = actionListJson.Begin(); actionIt != actionListJson.End(); actionIt++)
			{
				if (!actionIt->IsUint())
				{
					LOG_ERROR("Missing action id in 'action-list' in room");
					throw std::runtime_error("Invalid file 'home-info.json'");
				}

				Ref<Action> action = home->GetAction_(actionIt->GetUint());
				if (action == nullptr)
				{
					LOG_WARNING("Invalid action id in 'action-list' in room");
					continue;
				}

				AddAction(action);
			}
		}
	}
	void Room::Save(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator)
	{
		assert(json.IsObject());

		boost::shared_lock_guard lock(mutex);

		json.AddMember("name", rapidjson::Value(name.c_str(), name.size()), allocator);
		json.AddMember("id", rapidjson::Value(roomID), allocator);
		json.AddMember("type", rapidjson::Value(type), allocator);

		// Device
		rapidjson::Value deviceListJson = rapidjson::Value(rapidjson::kArrayType);
		for (WeakRef<Device>& deviceWeak : deviceList)
		{
			Ref<Device> device = deviceWeak.lock();
			if (device != nullptr)
				deviceListJson.PushBack(rapidjson::Value(device->GetDeviceID()), allocator);
		}
		json.AddMember("device-list", deviceListJson, allocator);

		// Action
		rapidjson::Value actionListJson = rapidjson::Value(rapidjson::kArrayType);
		for (WeakRef<Action>& actionWeak : actionList)
		{
			Ref<Action> action = actionWeak.lock();
			if (action != nullptr)
				actionListJson.PushBack(rapidjson::Value(action->GetActionID()), allocator);
		}
		json.AddMember("action-list", actionListJson, allocator);
	}
}
