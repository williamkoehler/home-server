#pragma once
#include "../common.h"

namespace home
{
	class Home;
	class DeviceManagerScript;

	class HOME_EXPORT DeviceManager
	{
	public:
		virtual inline Ref<Home> GetHome() = 0;

		virtual std::string GetName() = 0;
		virtual void SetName(std::string v) = 0;

		virtual uint32_t GetDeviceManagerID() = 0;

		virtual void AddUpdatable() = 0;

		virtual Ref<DeviceManagerScript> GetScript() = 0;
		template<class T>
		inline Ref<T> GetScript();
		virtual uint32_t GetScriptID() = 0;
	};

	template<class T>
	Ref<T> DeviceManager::GetScript()
	{
		return boost::dynamic_pointer_cast<T>(GetScript());
	}
}