#pragma once
#include "../common.h"
#include "Day.h"

namespace server
{
	class Calendar : public boost::enable_shared_from_this<Calendar>
	{
	private:
		boost::container::list<Day> dayList;

	public:
		Calendar();
		~Calendar();
		static Ref<Calendar> Create();
		static Ref<Calendar> GetInstance();
	};
}