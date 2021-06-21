#pragma once
#include "../common.h"
#include "Event.h"

namespace server
{
	class Day
	{
	private:
		boost::container::vector<Event> eventList;

	public:
		Day();
		~Day();
	};
}