#pragma once
#include "../common.h"

namespace home
{
	class Room;
	class Device;
	class DeviceManager;

	class DeviceScript;
	class DeviceManagerScript;

	class SignalManager
	{
	public:
		// Emit a signal that will be handled
		virtual void OnHomeChanged() = 0;

		virtual void OnRoomChanged(Ref<Room> room) = 0;

		virtual void OnDeviceChanged(Ref<Device> device) = 0;
		virtual void OnDeviceManagerChanged(Ref<DeviceManager> deviceManager) = 0;
		
		virtual void OnDevicePropertiesChanged(Ref<Device> device) = 0;
		virtual void OnDeviceManagerPropertiesChanged(Ref<DeviceManager> deviceManager) = 0;
	};
}