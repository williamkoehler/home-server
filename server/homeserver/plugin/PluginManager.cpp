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

	//! Timestamp
	void PluginManager::UpdateTimestamp()
	{
		const time_t ts = time(nullptr);
		timestamp = ts;
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
				library->get<home::RegisterPluginsFunction>("RegisterPlugins")(shared_from_this());

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

	bool PluginManager::RegisterDevicePlugin(const std::string& name, identifier_t pluginID, home::CreateDevicePluginFunction* createFunction)
	{
		boost::lock_guard lock(mutex);

		if (devicePluginList.count(pluginID))
		{
			LOG_ERROR("Device plugin already exists");
			return false;
		}

		devicePluginList[pluginID] = {
			name,
			pluginID,
			createFunction,
		};

		LOG_INFO("Successfully registered device plugin {0}:{1}", name, pluginID);
		return true;
	}
	Ref<home::DevicePlugin> PluginManager::CreateDevicePlugin(identifier_t pluginID)
	{
		boost::shared_lock_guard lock(mutex);

		// Find plugin
		boost::unordered::unordered_map<uint32_t, DevicePluginReference>::const_iterator it = devicePluginList.find(pluginID);
		if (it != devicePluginList.end())
			return it->second.createFunction();

		return nullptr;
	}

	bool PluginManager::RegisterDeviceControllerPlugin(const std::string& name, identifier_t pluginID, home::CreateDeviceControllerPluginFunction* createFunction)
	{
		boost::lock_guard lock(mutex);

		if (deviceControllerPluginList.count(pluginID))
		{
			LOG_ERROR("Device controller plugin already exists");
			return false;
		}

		deviceControllerPluginList[pluginID] = {
			name,
			pluginID,
			createFunction,
		};

		LOG_INFO("Successfully registered device controller plugin {0}:{1}", name, pluginID);
		return true;
	}
	Ref<home::DeviceControllerPlugin> PluginManager::CreateDeviceControllerPlugin(identifier_t pluginID)
	{
		boost::shared_lock_guard lock(mutex);

		// Find plugin
		boost::unordered::unordered_map<uint32_t, DeviceControllerPluginReference>::const_iterator it = deviceControllerPluginList.find(pluginID);
		if (it != deviceControllerPluginList.end())
			return it->second.createFunction();

		return nullptr;
	}
}