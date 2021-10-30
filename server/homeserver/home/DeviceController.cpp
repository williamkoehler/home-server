#include "DeviceController.hpp"
#include <xxHash/xxhash.h>
#include "../plugin/PluginManager.hpp"
#include "../database/Database.hpp"

namespace server
{
	DeviceController::DeviceController(const std::string& name, identifier_t controllerID, Ref<home::DeviceControllerPlugin> plugin, Ref<Room> room)
		: name(name), controllerID(controllerID), plugin(std::move(plugin)), room(std::move(room))
	{
	}
	DeviceController::~DeviceController()
	{
		plugin->Terminate();
	}

	std::string DeviceController::GetName()
	{
		boost::lock_guard lock(mutex);
		return name;
	}
	bool DeviceController::SetName(const std::string& v)
	{
		boost::lock_guard lock(mutex);
		if (Database::GetInstance()->UpdateDeviceControllerPropName(shared_from_this(), name, v))
		{
			name = v;
			return true;
		}
		return false;
	}

	Ref<Room> DeviceController::GetRoom()
	{
		boost::shared_lock_guard lock(mutex);
		return room;
	}
	bool DeviceController::SetRoom(Ref<Room> v)
	{
		boost::lock_guard lock(mutex);
		if (Database::GetInstance()->UpdateDeviceControllerPropRoom(shared_from_this(), room, v))
		{
			if (room != nullptr)
				room->RemoveDeviceController(shared_from_this());

			room = v;

			if (room != nullptr)
				room->AddDeviceController(shared_from_this());
			return true;
		}
		return false;
	}

	void DeviceController::Update(size_t cycle)
	{
		boost::lock_guard lock(pluginMutex);

		if (plugin->Update(cycle))
		{
			boost::lock_guard lock(mutex);
			plugin->cache.TakeSnapshot(snapshot);
		}
	}
}