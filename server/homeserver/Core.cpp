#include "Core.hpp"
#include "plugin/PluginManager.hpp"
#include "user/UserManager.hpp"
#include "signal/SignalManager.hpp"
#include "scripting/ScriptManager.hpp"
#include "home/Home.hpp"
#include "network/NetworkManager.hpp"
#include "io/DynamicResources.hpp"
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

		if (services.main != nullptr)
			services.main->stop();

		if (services.worker != nullptr)
			services.worker->stop();

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
				core->services.main = boost::make_shared<boost::asio::io_service>();
				if (core->additionalThreadCount)
					core->services.worker = boost::make_shared<boost::asio::io_service>(core->additionalThreadCount);
				else
					core->services.worker = core->services.main;

				core->maintenanceTimer = boost::make_shared<boost::asio::deadline_timer>(*core->services.main);
#ifdef _DEBUG
				core->maintenanceTimer->expires_from_now(boost::posix_time::hours(5));
#else
				core->maintenanceTimer->expires_from_now(boost::posix_time::hours(24));
#endif
				core->maintenanceTimer->async_wait(boost::bind(&Core::OnMaintenance, core.get(), boost::placeholders::_1));
			}

			// Starting signal handling
			{
				core->signalManager = SignalManager::Create();
				if (core->signalManager == nullptr)
					return nullptr;
			}

			// Starting scripting
			{
				core->scriptManager = scripting::ScriptManager::Create();
				if (core->scriptManager == nullptr)
					return nullptr;
			}

			// Starting home
			{
				core->userManager = UserManager::Create();
				if (core->userManager == nullptr)
					return nullptr;

				core->home = Home::Create();
				if (core->home == nullptr)
					return nullptr;
			}

			//Starting update cycle
			{
				core->updateTimer = boost::make_shared<boost::asio::deadline_timer>(*core->services.main);
				core->updateTimer->expires_from_now(boost::posix_time::seconds(1));
				core->updateTimer->async_wait(boost::bind(&Core::OnUpdate, core.get(), boost::placeholders::_1));
			}

			//Starting webserver
			core->networkManager = NetworkManager::Create(core->services.main, core->address, core->port);

			core->dynamicResources = DynamicResources::Create();
			if (core->dynamicResources == nullptr)
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

		LOG_INFO("Starting {0} additional threads(s)", additionalThreadCount);

		for (size_t i = 1; i < additionalThreadCount; i++)
			threads.create_thread(boost::bind(&Core::Worker, shared_from_this()));

		while (running)
		{
			try
			{
				services.main->run();
			}
			catch (std::exception e)
			{
				printf("Ooops!!! Something bad happend");
				LOG_ERROR("An exception was thrown and not catched: {0}", e.what());
			}
		}

		services.worker->stop();

		threads.join_all();

		try
		{
			userManager->Save();
			home->Save();
		}
		catch (std::exception) {}

		LOG_FLUSH();
	}

	void Core::Worker()
	{
		while (running)
		{
			try
			{
				services.worker->run();
				boost::this_thread::sleep_for(boost::chrono::milliseconds(200));
			}
			catch (std::exception e)
			{
				printf("Ooops!!! Something bad happend");
				LOG_ERROR("An exception was thrown and not catched: {0}", e.what());
			}
		}
	}

	void Core::OnMaintenance(boost::system::error_code error)
	{
		if (!error)
		{
			try
			{
				userManager->Save();
				home->Save();

				boost::lock_guard lock(mutex);

				maintenanceTimer->expires_from_now(boost::posix_time::hours(96));
				maintenanceTimer->async_wait(boost::bind(&Core::OnMaintenance, this, boost::placeholders::_1));

			}
			catch (std::exception)
			{
				LOG_ERROR("Error during maintenance. Please check for errors log. If the error is not fixed the server will not be able to save data.");

				boost::lock_guard lock(mutex);

				maintenanceTimer->expires_from_now(boost::posix_time::hours(2));
				maintenanceTimer->async_wait(boost::bind(&Core::OnMaintenance, this, boost::placeholders::_1));
			}

			LOG_FLUSH();
		}
	}

	// Change duty cycles
	void Core::IncreaseDutyCycles()
	{
		if (yieldTime < 50)
		{
			yieldTime -= 10;
			LOG_INFO("Yield time decreased to {0}ms", yieldTime);
		}
	}
	void Core::DecreaseDutyCycles()
	{
		if (yieldTime < 3000)
		{
			yieldTime += 5;
			LOG_INFO("Yield time increased to {0}ms", yieldTime);
		}
	}

	void Core::OnUpdate(boost::system::error_code error)
	{
		if (!error)
		{
			// Update home
			home->Update();

			boost::lock_guard lock(mutex);

			updateTimer->expires_from_now(boost::posix_time::milliseconds(yieldTime.load()));
			updateTimer->async_wait(boost::bind(&Core::OnUpdate, this, boost::placeholders::_1));
		}
	}

	//Shutdown
	void Core::Shutdown()
	{
		running = false;

		services.main->stop();
	}
}