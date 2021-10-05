#pragma once
#include "../common.hpp"
#include "../home/Home.hpp"
#include "../home/DeviceManager.hpp"
#include "../home/Device.hpp"

namespace home
{
	class Helper
	{
	public:
		//Script
		template<class T>
		static bool FindDeviceManager(const Ref<Home>& home, uint32_t managerID, Ref<DeviceManager>& manager, Ref<T>& script);
		template<class T>
		static bool FindDevice(const Ref<Home>& home, uint32_t deviceID, Ref<Device>& device, Ref<T>& script);
	};

	template<class T>
	inline bool Helper::FindDeviceManager(const Ref<Home>& home, uint32_t managerID, Ref<DeviceManager>& manager, Ref<T>& script)
	{
		manager = nullptr;
		script = nullptr;

		Ref<DeviceManager> m = home->GetDeviceManager(managerID);
		if (m == nullptr)
			return false;

		if (m->GetScriptID() != T::_getScriptID())
			return false;

		Ref<T> s = m->GetScript<T>();
		if (s == nullptr)
			return false;

		// Success
		manager = m;
		script = s;

		return true;
	}
	template<class T>
	inline bool Helper::FindDevice(const Ref<Home>& home, uint32_t deviceID, Ref<Device>& device, Ref<T>& script)
	{
		device = nullptr;
		script = nullptr;

		Ref<Device> d = home->GetDevice(deviceID);
		if (d == nullptr)
			return false;

		if (d->GetScriptID() != T::_getScriptID())
			return false;

		Ref<T> s = d->GetScript<T>();
		if (s == nullptr)
			return false;

		//Success
		device = d;
		script = s;

		return true;
	}
}