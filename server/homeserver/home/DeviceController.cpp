#include "DeviceController.hpp"
#include <xxHash/xxhash.h>
#include "../plugin/PluginManager.hpp"

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