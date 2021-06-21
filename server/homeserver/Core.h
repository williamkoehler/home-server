#pragma once
#include "common.h"

namespace scripting
{
	class DraftManager;
}

namespace server
{
	class SignalManager;
	class UserManager;
	class PluginManager;
	class Home;
	class NetworkManager;
	class DynamicResources;

	class JsonApi;

	// Core management
	// Contains all references to singletons like Home, UserManager, etc...
	// Core manages server threads and maintenance
	class Core : public boost::enable_shared_from_this<Core>
	{
	private:
		friend class JsonApi;

		boost::atomic_bool running = false;
		boost::shared_mutex mutex;

		//Server
		std::string name = "error-no-name";
		uint16_t port = 443;
		std::string address = "0.0.0.0";
		size_t additionalThreadCount = 1;
		Ref<SignalManager> signalManager = nullptr;
		Ref<UserManager> userManager = nullptr;
		Ref<scripting::DraftManager> draftManager = nullptr;
		Ref<Home> home = nullptr;
		Ref<NetworkManager> networkManager = nullptr;
		Ref<DynamicResources> dynamicResources = nullptr;

		struct
		{
			Ref<boost::asio::io_service> main = nullptr;
			Ref<boost::asio::io_service> worker = nullptr;
		} services;
		boost::thread_group threads;

		void Worker();

		Ref<boost::asio::deadline_timer> maintenanceTimer = nullptr;
		void OnMaintenance(boost::system::error_code error);

		// Workload
		Ref<boost::asio::deadline_timer> updateTimer = nullptr;
		std::atomic_size_t yieldTime = 2000;
		void OnUpdate(boost::system::error_code error);

		// IO
		void Load();
		void SaveDefault();

	public:
		Core();
		~Core();
		static Ref<Core> Create();
		static Ref<Core> GetInstance();

		inline bool IsRunning() const { return running; }

		inline std::string GetName()
		{
			boost::shared_lock_guard lock(mutex);
			return name;
		}

		inline Ref<boost::asio::io_service> GetService() const { return services.main; }
		inline Ref<boost::asio::io_service> GetWorkerService() const { return services.worker; }

		void Run();

		// Duty cycle
		void IncreaseDutyCycles();
		void DecreaseDutyCycles();

		// Shutdown
		void Shutdown();
	};

	std::string GetServerName();
}

