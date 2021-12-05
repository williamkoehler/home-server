#pragma once
#include "../common.hpp"
#include "../Plugin.hpp"
#include "Device.hpp"
#include "DeviceControllerPlugin.hpp"

namespace server
{
	class JsonApi;
	class Device;
}

namespace home
{
	struct DevicePluginDescription
	{
		std::string name;
		identifier_t pluginID;
		std::string description;
	};

	class DevicePlugin : public Plugin
	{
	public:
		friend class server::Device;

		virtual bool Initialize(Ref<Device> device) = 0;

		virtual bool Terminate(Ref<Device> device) = 0;
	};

	typedef Ref<DevicePlugin>(CreateDevicePluginFunction)();

	#define DEVICEPLUGIN_DESCRIPTION(class_, name_, desc_) \
		PLUGIN_DESCRIPTION(class_, name_) \
		static std::string GetPluginDescription_() { return desc_; } \
		static Ref<DevicePlugin> Create() { return boost::make_shared<class_>(); }
}