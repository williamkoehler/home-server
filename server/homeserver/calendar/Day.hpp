#pragma once
#include "../common.hpp"
#include "Event.hpp"

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