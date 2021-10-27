#pragma once
#include "../common.hpp"
#include "../home/Home.hpp"
#include "../home/DeviceController.hpp"
#include "../home/Device.hpp"

namespace home
{
	class Helper
	{
	public:
		//Script
		template<class T>
		static bool FindDeviceController(const Ref<Home>& home, identifier_t managerID, Ref<DeviceController>& manager, Ref<T>& script);
		template<class T>
		static bool FindDevice(const Ref<Home>& home, identifier_t deviceID, Ref<Device>& device, Ref<T>& script);
	};

	template<class T>
	inline bool Helper::FindDeviceController(const Ref<Home>& home, identifier_t managerID, Ref<DeviceController>& manager, Ref<T>& script)
	{
		manager = nullptr;
		script = nullptr;

		Ref<DeviceController> m = home->GetDeviceController(managerID);
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
	inline bool Helper::FindDevice(const Ref<Home>& home, identifier_t deviceID, Ref<Device>& device, Ref<T>& script)
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