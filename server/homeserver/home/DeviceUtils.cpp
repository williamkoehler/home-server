#include "Device.hpp"
#include "Home.hpp"

namespace server
{
	//! Timer
	Device::Timer::Timer(Ref<Device> device, home::TimerCallback<home::Device>* callback)
		: device(device), timer(*Home::GetInstance()->GetService()), callback(callback)
	{
	}
	Device::Timer::~Timer()
	{
	}

	void Device::Timer::TimerCallback(const boost::system::error_code& err)
	{
		if (!err)
		{
			// Invoke timer
			Invoke();

			timer.expires_from_now(boost::posix_time::seconds(interval));
			timer.async_wait(boost::bind(&Device::Timer::TimerCallback, this, boost::placeholders::_1));
		}
	}

	void Device::Timer::Start(size_t i)
	{
		interval = i;

		timer.expires_from_now(boost::posix_time::seconds(interval));
		timer.async_wait(boost::bind(&Device::Timer::TimerCallback, this, boost::placeholders::_1));
	}
	void Device::Timer::Invoke()
	{
		if (((home::TimerCallback<home::Device>*)callback)(device->plugin.get(), boost::reinterpret_pointer_cast<home::Device>(device)))
			device->TakeSnapshot();
	}
	void Device::Timer::Stop()
	{
		timer.cancel();
	}

	//! Event
	Device::Event::Event(Ref<Device> device, home::TimerCallback<home::Device>* callback)
		: device(device), callback(callback)
	{
	}
	Device::Event::~Event()
	{
	}

	void Device::Event::Invoke()
	{
		if (((home::TimerCallback<home::Device>*)callback)(device->plugin.get(), boost::reinterpret_pointer_cast<home::Device>(device)))
			device->TakeSnapshot();
	}
}