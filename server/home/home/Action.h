#pragma once
#include "../common.h"

namespace home
{
	class Home;

	class Action
	{
	public:
		virtual inline Ref<Home> GetHome() = 0;

		virtual inline std::string GetName() = 0;
		virtual inline void SetName(std::string v) = 0;

		virtual uint32_t GetActionID() = 0;
	};
}