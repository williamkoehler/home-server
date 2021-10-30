#pragma once
#include "common.hpp"
#include "Plugin.hpp"
#include "DevicePlugin.hpp"
#include "DeviceControllerPlugin.hpp"

namespace home
{
	class PluginManager
	{
	public:
		virtual bool RegisterDevicePlugin(DevicePluginDescription description, CreateDevicePluginFunction* createCallback) = 0;
		virtual bool RegisterDeviceControllerPlugin(DeviceControllerPluginDescription description, CreateDeviceControllerPluginFunction* createCallback) = 0;

		template<typename T>
		bool RegisterDevicePlugin();
		template<typename T>
		bool RegisterDeviceControllerPlugin();
	};

	template<typename T>
	bool PluginManager::RegisterDevicePlugin()
	{
		return RegisterDevicePlugin(DevicePluginDescription{ T::GetPluginName_(), T::GetPluginID_(), T::GetPluginDescription_() }, T::Create);
	}

	template<typename T>
	bool PluginManager::RegisterDeviceControllerPlugin()
	{
		return RegisterDeviceControllerPlugin(DeviceControllerPluginDescription{ T::GetPluginName_(), T::GetPluginID_(), T::GetPluginDescription_() }, T::Create);
	}

	typedef void (RegisterPluginsFunction)(Ref<PluginManager> pluginManager);
}