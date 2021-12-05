#include "DeviceController.hpp"
#include "Home.hpp"

namespace server
{
	//! Timer
	DeviceController::Timer::Timer(Ref<DeviceController> controller, home::TimerCallback<home::DeviceController>* callback)
		: controller(controller), timer(*Home::GetInstance()->GetService()), callback(callback)
	{
	}
	DeviceController::Timer::~Timer()
	{
	}

	void DeviceController::Timer::TimerCallback(const boost::system::error_code& err)
	{
		if (!err)
		{
			// Invoke timer
			Invoke();

			timer.expires_from_now(boost::posix_time::seconds(interval));
			timer.async_wait(boost::bind(&DeviceController::Timer::TimerCallback, this, boost::placeholders::_1));
		}
	}

	void DeviceController::Timer::Start(size_t i)
	{
		interval = i;

		timer.expires_from_now(boost::posix_time::seconds(interval));
		timer.async_wait(boost::bind(&DeviceController::Timer::TimerCallback, this, boost::placeholders::_1));
	}
	void DeviceController::Timer::Invoke()
	{
		if (((home::TimerCallback<home::DeviceController>*)callback)(controller->plugin.get(), boost::reinterpret_pointer_cast<home::DeviceController>(controller)))
			controller->TakeSnapshot();
	}
	void DeviceController::Timer::Stop()
	{
		timer.cancel();
	}

	//! Event
	DeviceController::Event::Event(Ref<DeviceController> controller, home::EventCallback<home::DeviceController>* callback)
		: controller(controller), callback(callback)
	{
	}
	DeviceController::Event::~Event()
	{
	}

	void DeviceController::Event::Invoke()
	{
		if (((home::TimerCallback<home::DeviceController>*)callback)(controller->plugin.get(), boost::reinterpret_pointer_cast<home::DeviceController>(controller)))
			controller->TakeSnapshot();
	}
}