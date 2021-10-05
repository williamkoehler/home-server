#pragma once
#include "../common.hpp"
#include "Day.hpp"

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