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

		std::string name = "error-no-name";

		// Networking
		std::string externalUrl;
		std::string address = "0.0.0.0";
		uint16_t port = 443;

		// Threading
		size_t threadCount = 1;
		boost::thread_group threads;
		Ref<boost::asio::io_service> service = nullptr;
		Ref<boost::asio::io_service::work> work = nullptr;

		// Components
		Ref<Database> database = nullptr;
		Ref<NetworkManager> networkManager = nullptr;
		Ref<UserManager> userManager = nullptr;
		Ref<ScriptManager> scriptManager = nullptr;
		Ref<Home> home = nullptr;

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

		inline const std::string& GetName() { return name; }

		// Networking
		inline const std::string& GetAddress() { return address; } 
		inline uint16_t GetPort() { return port; }
		inline const std::string& GetExternalUrl() { return externalUrl; }

		inline Ref<boost::asio::io_service> GetService() const { return service; }

		void Run();

		// Shutdown
		void Shutdown();
	};

	std::string GetServerName();
}

