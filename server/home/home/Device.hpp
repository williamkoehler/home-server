#pragma once
#include "../common.hpp"

namespace home
{
	class Home;
	class DeviceManager;

	class DeviceScript;

	class Device
	{
	public:
		virtual inline Ref<Home> GetHome() = 0;

		virtual inline std::string GetName() = 0;
		virtual inline void SetName(std::string v) = 0;

		virtual inline uint32_t GetDeviceID() = 0;

		virtual void AddUpdatable() = 0;

		virtual inline Ref<DeviceScript> GetScript() = 0;
		template<class T>
		inline Ref<T> GetScript();
		virtual inline uint32_t GetScriptID() = 0;
	};

	template<class T>
	Ref<T> Device::GetScript()
	{
		return boost::dynamic_pointer_cast<T>(GetScript());
	}
}