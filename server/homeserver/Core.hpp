#pragma once
#include "common.hpp"

namespace server
{
	class Database;
	class UserManager;
	class ScriptManager;
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
		size_t threadCount = 1;
		Ref<Database> database = nullptr;
		Ref<UserManager> userManager = nullptr;
		Ref<ScriptManager> scriptManager = nullptr;
		Ref<Home> home = nullptr;
		Ref<NetworkManager> networkManager = nullptr;

		Ref<boost::asio::io_service> service = nullptr;
		boost::thread_group threads;

		void Worker();

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

		inline Ref<boost::asio::io_service> GetService() const { return service; }

		void Run();

		// Shutdown
		void Shutdown();
	};

	std::string GetServerName();
}

