#pragma once
#include "../common.hpp"

namespace home
{
	class DeviceManager;
	class Room;
	class Device;
	class Action;

	class Home
	{
	public:
		virtual inline size_t GetRoomCount() = 0;
		virtual Ref<Room> GetRoom(uint32_t roomID) = 0;

		virtual inline size_t GetDeviceCount() = 0;
		virtual Ref<home::Device> GetDevice(uint32_t deviceID) = 0;

		virtual size_t GetDeviceManagerCount() = 0;
		virtual Ref<DeviceManager> GetDeviceManager(uint32_t managerID) = 0;

		virtual size_t GetActionCount() = 0;
		virtual Ref<Action> GetAction(uint32_t actionID) = 0;
	};
}