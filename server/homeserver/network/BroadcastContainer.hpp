#pragma once
#include "../common.hpp"

namespace server
{
	class WSSession;

	class BroadcastContainer
	{
	private:
		friend class WSSession;

		// JSON Documents containing broadcasts

		// set-rooms?info
		// set-devices?info
		// set-devicemanagers?info
		// set-devices?state
		// set-devicemanagers?state

	};
}