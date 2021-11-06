#include "PluginManager.hpp"

namespace server
{
	boost::weak_ptr<PluginManager> instancePluginManager;

	PluginManager::PluginManager()
	{
	}
	PluginManager::~PluginManager()
	{
		// Clear plugin list
		devicePluginList.clear();

		// Clear loaded libraries
		libraryList.clear();
	}
	Ref<PluginManager> PluginManager::Create()
	{
		if (!instancePluginManager.expired())
			return Ref<PluginManager>(instancePluginManager);

		Ref<PluginManager> pluginManager = boost::make_shared<PluginManager>();
		instancePluginManager = pluginManager;

		try
		{
			pluginManager->Load();

			// Update snapshot
			pluginManager->TakeSnapshot();
		}
		catch (std::exception e)
		{
			return nullptr;
		}

		LOG_FLUSH();

		return pluginManager;
	}
	Ref<PluginManager> PluginManager::GetInstance()
	{
		return Ref<PluginManager>(instancePluginManager);
	}

	void PluginManager::TakeSnapshot()
	{
		boost::shared_lock_guard lock(mutex);

		rapidjson::Document::AllocatorType& allocator = snapshot.GetAllocator();

		// Clear memory
		snapshot.SetNull();
		allocator.Clear();

		// Fill document
		snapshot.SetObject();

		// Update device plugins
		rapidjson::Value devicePluginListJson = rapidjson::Value(rapidjson::kArrayType);

		for (const std::pair<const identifier_t, DevicePluginReference>& devicePlugin : devicePluginList)
		{
			rapidjson::Value devicePluginJson = rapidjson::Value(rapidjson::kObjectType);

			home::DevicePluginDescription description = devicePlugin.second.description;

			devicePluginJson.AddMember("name", rapidjson::Value(description.name.c_str(), description.name.size(), allocator), allocator);
			devicePluginJson.AddMember("pluginid", rapidjson::Value(description.pluginID), allocator);
			devicePluginJson.AddMember("description", rapidjson::Value(description.description.data(), description.description.size(), allocator), allocator);

			devicePluginListJson.PushBack(devicePluginJson, allocator);
		}

		snapshot.AddMember("devices", devicePluginListJson, allocator);

		// Update device controller plugins
		rapidjson::Value deviceControllerPluginListJson = rapidjson::Value(rapidjson::kArrayType);

		for (const std::pair<const identifier_t, DeviceControllerPluginReference>& deviceControllerPlugin : deviceControllerPluginList)
		{
			rapidjson::Value devicePluginJson = rapidjson::Value(rapidjson::kObjectType);

			home::DeviceControllerPluginDescription description = deviceControllerPlugin.second.description;

			devicePluginJson.AddMember("name", rapidjson::Value(description.name.c_str(), description.name.size(), allocator), allocator);
			devicePluginJson.AddMember("pluginid", rapidjson::Value(description.pluginID), allocator);
			devicePluginJson.AddMember("description", rapidjson::Value(description.description.data(), description.description.size(), allocator), allocator);

			deviceControllerPluginListJson.PushBack(devicePluginJson, allocator);
		}

		snapshot.AddMember("devicecontrollers", deviceControllerPluginListJson, allocator);
	}

	void PluginManager::LoadPlugin(std::string name)
	{
		boost::filesystem::path filePath = boost::filesystem::weakly_canonical(boost::filesystem::path("./plugins/" + name));

		Ref<boost::dll::shared_library> library = boost::make_shared<boost::dll::shared_library>();

		boost::system::error_code ec;
		library->load(filePath, boost::dll::load_mode::append_decorations, ec);

		if (ec)
		{
			LOG_ERROR("Load or open plugin library '{0}'", name);
			throw std::runtime_error("Load plugin");
		}

		if (library->has("RegisterPlugins"))
		{
			try
			{
				LOG_INFO("Registering plugin '{0}'", name);
				library->get<home::RegisterPluginsFunction>("RegisterPlugins")(boost::static_pointer_cast<home::PluginManager>(shared_from_this()));

				boost::lock_guard lock(mutex);
				libraryList.push_back(library);
			}
			catch (std::exception)
			{
				LOG_ERROR("Call function 'RegisterPlugins' in plugin library '{0}'", name);
				throw std::runtime_error("Initialize plugin");
			}
		}
		else
		{
			LOG_ERROR("Missing 'RegisterPlugins' function in plugin library '{0}'", name);
			throw std::runtime_error("Initialize plugin");
		}
	}

	bool PluginManager::RegisterDevicePlugin(home::DevicePluginDescription description, home::CreateDevicePluginFunction* createFunction)
	{
		boost::lock_guard lock(mutex);

		if (description.pluginID == 0)
		{
			LOG_ERROR("Device plugin has invalid description");
			return false;
		}

		if (devicePluginList.count(description.pluginID))
		{
			LOG_ERROR("Device plugin already exists");
			return false;
		}

		devicePluginList[description.pluginID] = {
			description,
			createFunction,
		};

		LOG_INFO("Successfully registered device plugin {0}:{1}", description.name, description.pluginID);
		return true;
	}
	Ref<home::DevicePlugin> PluginManager::CreateDevicePlugin(identifier_t pluginID)
	{
		boost::shared_lock_guard lock(mutex);

		// Find plugin
		boost::unordered::unordered_map<identifier_t, DevicePluginReference>::const_iterator it = devicePluginList.find(pluginID);
		if (it != devicePluginList.end())
			return it->second.createFunction();

		return nullptr;
	}

	bool PluginManager::RegisterDeviceControllerPlugin(home::DeviceControllerPluginDescription description, home::CreateDeviceControllerPluginFunction* createFunction)
	{
		boost::lock_guard lock(mutex);

		if (description.pluginID == 0)
		{
			LOG_ERROR("Device controller has invalid description");
			return false;
		}

		if (deviceControllerPluginList.count(description.pluginID))
		{
			LOG_ERROR("Device controller plugin already exists");
			return false;
		}

		deviceControllerPluginList[description.pluginID] = {
			description,
			createFunction,
		};

		LOG_INFO("Successfully registered device controller plugin {0}:{1}", description.name, description.pluginID);
		return true;
	}
	Ref<home::DeviceControllerPlugin> PluginManager::CreateDeviceControllerPlugin(identifier_t pluginID)
	{
		boost::shared_lock_guard lock(mutex);

		// Find plugin
		boost::unordered::unordered_map<identifier_t, DeviceControllerPluginReference>::const_iterator it = deviceControllerPluginList.find(pluginID);
		if (it != deviceControllerPluginList.end())
			return it->second.createFunction();

		return nullptr;
	}
}