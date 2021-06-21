#pragma once
#include "common.h"
#include "Script.h"
#include "home/DeviceScript.h"
#include "home/DeviceManagerScript.h"

namespace home
{
	class HOME_EXPORT Plugin
	{
	protected:
		virtual void RegisterDeviceScript(uint32_t scriptID, DeviceScriptDescription& scriptDescription) = 0;
		virtual void RegisterDeviceManagerScript(uint32_t scriptID, DeviceManagerScriptDescription& scriptDescription) = 0;

		Plugin();
		~Plugin();
	public:
		template<typename T>
		void RegisterDeviceScript();

		template<typename T>
		void RegisterDeviceManagerScript();
	};

	template<typename T>
	void Plugin::RegisterDeviceScript()
	{
		const uint32_t scriptID = T::_getScriptID();

		DeviceScriptDescription desc;

		T::OnRegister(&desc);

		desc.createFunction = &T::_createDeviceScript;
		desc.name = T::_getScriptName();
		desc.type = T::_getType();
		desc.scriptID = scriptID;

		RegisterDeviceScript(scriptID, desc);
		LOG_INFO("Registering device script '{0}' id {1}", T::_getScriptName(), T::_getScriptID());
	}

	template<typename T>
	void Plugin::RegisterDeviceManagerScript()
	{
		const uint32_t scriptID = T::_getScriptID();

		DeviceManagerScriptDescription desc;

		T::OnRegister(&desc);

		desc.createFunction = &T::_createDeviceManagerScript;
		desc.name = T::_getScriptName();
		desc.scriptID = scriptID;

		RegisterDeviceManagerScript(scriptID, desc);
		LOG_INFO("Registering device manager script '{0}' id {1}", T::_getScriptName(), T::_getScriptID());
	}

	typedef void (RegisterPluginFunction)(Ref<Plugin> plugin);
}