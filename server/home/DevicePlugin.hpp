#pragma once
#include "common.hpp"
#include "Plugin.hpp"
#include "DeviceControllerPlugin.hpp"

namespace home
{
	struct DevicePluginDescription
	{
		std::string name;
		identifier_t pluginID;
		std::string description;
	};

	class DeviceCache : public DeviceControllerCache
	{
	protected:

	public:
		template<typename Type>
		void SetProperty(uint32_t id, Type value);

		virtual void TakeSnapshot(rapidjson::Document& document) override
		{
			DeviceControllerCache::TakeSnapshot(document);
		}
	};

	class DevicePlugin : public Plugin<DeviceCache>
	{
	public:
		friend class server::Device;

		virtual bool Update(Ref<DevicePlugin> controller, size_t cycle) = 0;
	};

	typedef Ref<DevicePlugin>(CreateDevicePluginFunction)();

	#define DEVICEPLUGIN_DESCRIPTION(class_, name_, desc_) \
		PLUGIN_DESCRIPTION(class_, name_) \
		static std::string GetPluginDescription_() { return desc_; } \
		static Ref<DevicePlugin> Create() { return boost::make_shared<class_>(); }
}