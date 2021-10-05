#pragma once
#include "../common.hpp"

namespace home
{
	class Home;
	class Device;
	class Action;

	class Room
	{
	public:
		virtual inline Ref<home::Home> GetHome() = 0;

		virtual inline uint32_t GetRoomID() = 0;

		virtual inline size_t GetDeviceCount() = 0;
		virtual inline size_t GetActionCount() = 0;
	};
}