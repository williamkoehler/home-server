#include "Device.hpp"
#include <xxHash/xxhash.h>
#include "../plugin/PluginManager.hpp"
#include "../database/Database.hpp"

namespace server
{
	Device::Device(const std::string& name, identifier_t deviceID, Ref<home::DevicePlugin> plugin, Ref<DeviceController> controller, Ref<Room> room)
		: name(name), deviceID(deviceID), plugin(std::move(plugin)), controller(std::move(controller)), room(std::move(room))
	{
	}
	Device::~Device()
	{
		plugin->Terminate();
	}


	std::string Device::GetName()
	{
		boost::lock_guard lock(mutex);
		return name;
	}
	bool Device::SetName(const std::string& v)
	{
		boost::lock_guard lock(mutex);
		if (Database::GetInstance()->UpdateDevicePropName(shared_from_this(), name, v))
		{
			name = v;
			return true;
		}
		return false;
	}

	Ref<Room> Device::GetRoom()
	{
		boost::shared_lock_guard lock(mutex);
		return room;
	}
	bool Device::SetRoom(Ref<Room> v)
	{
		boost::lock_guard lock(mutex);
		if (Database::GetInstance()->UpdateDevicePropRoom(shared_from_this(), room, v))
		{
			if (room != nullptr)
				room->RemoveDevice(shared_from_this());

			room = v;

			if (room != nullptr)
				room->AddDevice(shared_from_this());
			return true;
		}
		return false;
	}

	void Device::Update(size_t cycle)
	{
		boost::lock_guard lock(pluginMutex);

		if (plugin->Update(nullptr, cycle))
		{
			boost::lock_guard lock(mutex);
			plugin->cache.TakeSnapshot(snapshot);
		}
	}
}