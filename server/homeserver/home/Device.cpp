#include "Device.hpp"
#include <xxHash/xxhash.h>
#include "../plugin/PluginManager.hpp"

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