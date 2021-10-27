#include "Core.hpp"
#include "plugin/PluginManager.hpp"
#include "database/Database.hpp"
#include "user/UserManager.hpp"
#include "scripting/ScriptManager.hpp"
#include "home/Home.hpp"
#include "network/NetworkManager.hpp"
#include "tools.hpp"

namespace server
{
	boost::weak_ptr<Core> instanceCore;

	std::string GetServerName()
	{
		return Ref<Core>(instanceCore)->GetName();
	}

	Core::Core()
	{
	}
	Core::~Core()
	{
		running = false;

		if (service != nullptr)
			service->stop();

		LOG_INFO("Terminating core server");
	}
	Ref<Core> Core::Create()
	{
		if (!instanceCore.expired())
			return Ref<Core>(instanceCore);

		Ref<Core> core = boost::make_shared<Core>();
		instanceCore = core;

		LOG_INFO("Initializing core server");

		try
		{
			core->Load();

			LOG_INFO("This server is called '{0}'", core->name);

			//Init asio
			{
				core->service = boost::make_shared<boost::asio::io_service>(core->threadCount);
			}

			// Initialize database
			core->database = Database::Create();
			if (core->database == nullptr)
				return nullptr;

			// Initialize home
			{
				core->userManager = UserManager::Create();
				if (core->userManager == nullptr)
					return nullptr;

				core->home = Home::Create();
				if (core->home == nullptr)
					return nullptr;

				core->scriptManager = ScriptManager::Create();
				if (core->scriptManager == nullptr)
					return nullptr;
			}

			// Initialize webserver
			core->networkManager = NetworkManager::Create(core->service, core->address, core->port);
			if (core->networkManager == nullptr)
				return nullptr;
		}
		catch (std::exception)
		{
			return nullptr;
		}

		LOG_FLUSH();

		return core;
	}

	Ref<Core> Core::GetInstance()
	{
		return Ref<Core>(instanceCore);
	}

	void Core::Run()
	{
		if (running)
			return;

		running = true;

		LOG_INFO("Starting {0} threads(s)", threadCount);

		for (size_t i = 1; i < threadCount; i++)
			threads.create_thread(boost::bind(&Core::Worker, shared_from_this()));

		while (running)
		{
			try
			{
				service->run();
				boost::this_thread::sleep_for(boost::chrono::milliseconds(200));
			}
			catch (std::exception e)
			{
				printf("Ooops!!! Something bad happend");
				LOG_ERROR("An exception was thrown and not catched: {0}", e.what());
			}
		}

		service->stop();

		threads.join_all();

		try
		{
			userManager->Save();
		}
		catch (std::exception) { }

		LOG_FLUSH();
	}

	void Core::Worker()
	{
		while (running)
		{
			try
			{
				service->run();
				boost::this_thread::sleep_for(boost::chrono::milliseconds(200));
			}
			catch (std::exception e)
			{
				printf("Ooops!!! Something bad happend");
				LOG_ERROR("An exception was thrown and not catched: {0}", e.what());
			}
		}
	}

	//Shutdown
	void Core::Shutdown()
	{
		running = false;

		service->stop();
	}
}