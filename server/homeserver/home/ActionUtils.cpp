#include "Action.hpp"
#include "Home.hpp"

namespace server
{
	//! Timer
	Action::Timer::Timer(Ref<Action> action, const std::string& callback)
		: action(action), timer(*Home::GetInstance()->GetService()), callback(callback)
	{
	}
	Action::Timer::~Timer()
	{
	}

	void Action::Timer::TimerCallback(const boost::system::error_code& err)
	{
		if (!err)
		{
			// Invoke timer
			Invoke();

			timer.expires_from_now(boost::posix_time::seconds(interval));
			timer.async_wait(boost::bind(&Action::Timer::TimerCallback, this, boost::placeholders::_1));
		}
	}

	void Action::Timer::Start(size_t i)
	{
		interval = i;

		timer.expires_from_now(boost::posix_time::seconds(interval));
		timer.async_wait(boost::bind(&Action::Timer::TimerCallback, this, boost::placeholders::_1));
	}
	void Action::Timer::Invoke()
	{
		if (action->script->Invoke(callback))
			action->TakeSnapshot();
	}
	void Action::Timer::Stop()
	{
		timer.cancel();
	}

	//! Event
	Action::Event::Event(Ref<Action> action, const std::string& callback)
		: action(action), callback(callback)
	{
	}
	Action::Event::~Event()
	{
	}

	void Action::Event::Invoke()
	{
		if (action->script->Invoke(callback))
			action->TakeSnapshot();
	}
}