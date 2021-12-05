#pragma once
#include "../common.hpp"
#include "../Plugin.hpp"
#include "DeviceController.hpp"

namespace server
{
	class JsonApi;
	class DeviceController;
}

namespace home
{
	struct DeviceControllerPluginDescription
	{
		std::string name;
		identifier_t pluginID;
		std::string description;
	};

	class DeviceControllerPlugin : public Plugin
	{
	public:
		friend class server::DeviceController;

		virtual bool Initialize(Ref<DeviceController> controller) = 0;

		virtual bool Terminate(Ref<DeviceController> controller) = 0;
	};

	typedef Ref<DeviceControllerPlugin>(CreateDeviceControllerPluginFunction)();

#define DEVICECONTROLLERPLUGIN_DESCRIPTION(class_, name_, desc_) \
		PLUGIN_DESCRIPTION(class_, name_) \
		static std::string GetPluginDescription_() { return desc_; } \
		static Ref<DeviceControllerPlugin> Create() { return boost::make_shared<class_>(); }
}