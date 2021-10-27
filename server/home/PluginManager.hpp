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
		virtual bool RegisterDevicePlugin(const std::string& name, identifier_t pluginID, CreateDevicePluginFunction* createCallback) = 0;
		virtual bool RegisterDeviceControllerPlugin(const std::string& name, identifier_t pluginID, CreateDeviceControllerPluginFunction* createCallback) = 0;
		
		template<typename T>
		bool RegisterDevicePlugin();
		template<typename T>
		bool RegisterDeviceControllerPlugin();
	};

	template<typename T>
	bool PluginManager::RegisterDevicePlugin()
	{
		return RegisterDevicePlugin(T::GetPluginName_(), T::GetPluginID_(), T::Create);
	}

	template<typename T>
	bool PluginManager::RegisterDeviceControllerPlugin()
	{
		return RegisterDeviceControllerPlugin(T::GetPluginName_(), T::GetPluginID_(), T::Create);
	}

	typedef void (RegisterPluginsFunction)(Ref<PluginManager> pluginManager);
}