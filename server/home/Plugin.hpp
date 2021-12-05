#pragma once
#include "common.hpp"

namespace server
{
	class Device;
	class DeviceController;
}

namespace home
{
	class Plugin
	{
	protected:
		friend class server::Device;
		friend class server::DeviceController;

	public:
		virtual std::string GetPluginName() = 0;
		virtual uint32_t GetPluginID() = 0;
	};

#define PLUGIN_DESCRIPTION(class_, name_) \
		static std::string GetPluginName_() { return name_; } \
		static uint32_t GetPluginID_() { return XXH32(name_, strlen(name_), 0x504c5547); } \
		virtual std::string GetPluginName() override { return GetPluginName_(); } \
		virtual uint32_t GetPluginID() override { return GetPluginID_(); }
}