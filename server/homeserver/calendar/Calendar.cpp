#include "Calendar.h"

namespace server
{
	boost::weak_ptr<Calendar> instanceCalendar;

	Calendar::Calendar()
	{
	}
	Calendar::~Calendar()
	{
	}
	Ref<Calendar> Calendar::Create()
	{
		if (!instanceCalendar.expired())
			return Ref<Calendar>(instanceCalendar);

		Ref<Calendar> calendar = boost::make_shared<Calendar>();
		instanceCalendar = calendar;

		try
		{

		}
		catch (std::exception)
		{
			return nullptr;
		}

		return calendar;
	}
	Ref<Calendar> Calendar::GetInstance()
	{
		return Ref<Calendar>(instanceCalendar);
	}
}