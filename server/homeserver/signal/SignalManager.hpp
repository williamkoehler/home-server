#pragma once
#include "../common.hpp"
#include <signal/SignalManager.hpp>

namespace server
{
	class SignalManager : public home::SignalManager, public boost::enable_shared_from_this<SignalManager>
	{
	private:
		// Only used in HandleSingals !
		boost::posix_time::ptime startTime;
		boost::atomic_size_t signalCount;
	public:
		SignalManager();
		~SignalManager();
		static Ref<SignalManager> Create();
		static Ref<SignalManager> GetInstance();

		//
		void ResetCounter();
		void Update();

		// Emit a signal that will be handled
		void OnHomeChanged() override;

		void OnRoomChanged(Ref<home::Room> room) override;

		void OnDeviceChanged(Ref<home::Device> device) override;
		void OnDevicePropertiesChanged(Ref<home::Device> device) override;

		void OnDeviceManagerChanged(Ref<home::DeviceManager> deviceManager) override;
		void OnDeviceManagerPropertiesChanged(Ref<home::DeviceManager> deviceManager) override;
	};
}