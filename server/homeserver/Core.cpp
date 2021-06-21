#include "Core.h"
#include "signal/SignalManager.h"
#include "user/UserManager.h"
#include "plugin/PluginManager.h"
#include "scripting/DraftManager.h"
#include "home/Home.h"
#include "home/Room.h"
#include "home/Device.h"
#include "home/DeviceManager.h"
#include "signal/SignalManager.h"
#include "io/DynamicResources.h"
#include "tools.h"
#include "network/NetworkManager.h"
#include "Version.h"
#include "tools/EMail.h"

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
				core->draftManager = scripting::DraftManager::Create();
				if (core->draftManager == nullptr)
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
				boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
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

	//IO
	void Core::Load()
	{
		LOG_INFO("Loading core information from 'core-info.json'");

		FILE* file = fopen("core-info.json", "r");
		if (file == nullptr)
		{
			LOG_ERROR("Failing to open/find 'core-info.json'");
			if (!boost::filesystem::exists("core-info.json"))
				SaveDefault();
			throw std::runtime_error("Open/find file 'core-info.json'");
		}

		boost::lock_guard lock(mutex);

		char buffer[RAPIDJSON_BUFFER_SIZE_SMALL];
		rapidjson::FileReadStream stream(file, buffer, sizeof(buffer));

		rapidjson::Document document;
		if (document.ParseStream(stream).HasParseError() || !document.IsObject())
		{
			LOG_ERROR("Failing to read 'core-info.json'");
			throw std::runtime_error("Read file 'core-info.json'");
		}

		//Load server name
		rapidjson::Value::MemberIterator nameIt = document.FindMember("name");
		if (nameIt != document.MemberEnd() && nameIt->value.IsString())
			name.assign(nameIt->value.GetString(), nameIt->value.GetStringLength());
		else
		{
			name = "My Home Automation server";
			LOG_WARNING("Missing 'name' in 'core-info.json'. Name will be set to 'My Home Automation server v{0}'.", SERVER_VERSION);
		}

		//Load web server port
		rapidjson::Value::MemberIterator portIt = document.FindMember("port");
		if (portIt != document.MemberEnd() && portIt->value.IsUint())
			port = portIt->value.GetUint();
		else
		{
			port = 443;
			LOG_WARNING("Missing 'port' in 'core-info.json'. Port will be set to default '443'.");
		}

		//Load web server address
		rapidjson::Value::MemberIterator addressIt = document.FindMember("address");
		if (addressIt != document.MemberEnd() && addressIt->value.IsString())
			address = std::string(addressIt->value.GetString(), addressIt->value.GetStringLength());
		else
		{
			address = "0.0.0.0";
			LOG_WARNING("Missing 'address' in 'core-info.json'. Address will be set to default '0.0.0.0'.");
		}

		LOG_INFO("Server address is {0}:{1}", address, port);

		//Load thread counts
		rapidjson::Value::MemberIterator additionalThreadCountIt = document.FindMember("additional-thread-count");
		if (additionalThreadCountIt != document.MemberEnd() && additionalThreadCountIt->value.IsUint())
			additionalThreadCount = additionalThreadCountIt->value.GetUint();
		else
		{
			additionalThreadCount = boost::thread::hardware_concurrency() - 1;
			LOG_WARNING("Missing 'main-thread-count' in 'core-info.json'. Default value will be used.");
		}

		if (additionalThreadCount == 0)
		{
			LOG_WARNING("No additional worker thread is used. This can cause some slowdowns");
		}

		LOG_INFO("Using {0} additional thread(s)", additionalThreadCount);

		//Load EMail
		rapidjson::Value::MemberIterator emailIt = document.FindMember("email-service");
		if (emailIt != document.MemberEnd() && emailIt->value.IsObject())
		{
			rapidjson::Value::MemberIterator addressIt = emailIt->value.FindMember("server-address");
			rapidjson::Value::MemberIterator portIt = emailIt->value.FindMember("server-port");
			rapidjson::Value::MemberIterator userAddressIt = emailIt->value.FindMember("user-address");
			rapidjson::Value::MemberIterator userPasswordIt = emailIt->value.FindMember("user-password");
			rapidjson::Value::MemberIterator recipientListIt = emailIt->value.FindMember("recipients");

			if (addressIt != document.MemberEnd() && addressIt->value.IsString() &&
				portIt != document.MemberEnd() && portIt->value.IsUint() &&
				userAddressIt != document.MemberEnd() && userAddressIt->value.IsString() &&
				userPasswordIt != document.MemberEnd() && userPasswordIt->value.IsString() &&
				recipientListIt != document.MemberEnd() && recipientListIt->value.IsArray())
			{
				Ref<EMail> email = EMail::Create(addressIt->value.GetString(), portIt->value.GetUint(),
					userAddressIt->value.GetString(), userPasswordIt->value.GetString());

				rapidjson::Value& recipientListJson = recipientListIt->value;
				for (rapidjson::Value::ValueIterator recipientIt = recipientListJson.Begin(); recipientIt != recipientListJson.End(); recipientIt++)
					if (recipientIt->IsString())
						email->AddRecipient(recipientIt->GetString());

				rapidjson::Value::MemberIterator sendEMailAtLaunchIt = emailIt->value.FindMember("send-email-at-launch");
				if (sendEMailAtLaunchIt != emailIt->value.MemberEnd() && sendEMailAtLaunchIt->value.IsBool())
					if (sendEMailAtLaunchIt->value.GetBool())
						email->Send("HomeAutomation Server", "Hello, \n\nThe home automation server has just started!\n\nNote that this email is sent automatically as soon as the server is started!\nYou can disable this feature in 'core-info.json'.");
			}
		}

		fclose(file);
	}
	void Core::SaveDefault()
	{
		boost::shared_lock_guard lock(mutex);

		LOG_INFO("Saving default core information to 'core-info.json'");

		//Create json
		rapidjson::Document document = rapidjson::Document(rapidjson::kObjectType);

		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		document.AddMember("name", rapidjson::Value("Home Automation Server", 22), allocator);
		document.AddMember("port", rapidjson::Value(443), allocator);
		document.AddMember("address", rapidjson::Value("0.0.0.0", 7), allocator);
		{
			const size_t maxThreadCount = std::clamp(boost::thread::hardware_concurrency(), static_cast<uint32_t>(1), static_cast<uint32_t>(11));
			document.AddMember("additional-thread-count", rapidjson::Value(maxThreadCount - 1), allocator);
		}
		document.AddMember("use-ssh", rapidjson::Value(false), allocator);

		{
			rapidjson::Value emailServiceJson = rapidjson::Value(rapidjson::kObjectType);

			emailServiceJson.AddMember("server-address", rapidjson::Value("server address"), allocator);
			emailServiceJson.AddMember("server-port", rapidjson::Value("465"), allocator);
			emailServiceJson.AddMember("user-address", rapidjson::Value("email.address@domain"), allocator);
			emailServiceJson.AddMember("user-password", rapidjson::Value("password"), allocator);
			emailServiceJson.AddMember("recipients", rapidjson::Value(rapidjson::kArrayType), allocator);

			document.AddMember("email-service", emailServiceJson, allocator);
		}

		//Save to file
		FILE* file = fopen("core-info.json", "w");
		if (file == nullptr)
		{
			LOG_ERROR("Failing to open/find 'core-info.json'");
			throw std::runtime_error("Open/find file 'core-info.json'");
		}

		char buffer[RAPIDJSON_BUFFER_SIZE_SMALL];
		rapidjson::FileWriteStream stream(file, buffer, sizeof(buffer));

		rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer = rapidjson::PrettyWriter<rapidjson::FileWriteStream>(stream);
		document.Accept(writer);

		fclose(file);
	}
}