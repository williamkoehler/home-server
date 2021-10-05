#include "PluginManager.hpp"

namespace server
{
	boost::weak_ptr<PluginManager> instancePluginManager;

	PluginManager::PluginManager()
	{
	}
	PluginManager::~PluginManager()
	{
		deviceManagerScriptList.clear();
		deviceScriptList.clear();

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

		if (library->has("RegisterPlugin"))
		{
			try
			{
				LOG_INFO("Registering plugin '{0}'", name);
				library->get<home::RegisterPluginFunction>("RegisterPlugin")(shared_from_this());

				boost::lock_guard lock(mutex);
				libraryList[filePath] = library;
			}
			catch (std::exception)
			{
				LOG_ERROR("Call function 'RegisterPlugin' in plugin library '{0}'", name);
				throw std::runtime_error("Initialize plugin");
			}
		}
		else
		{
			LOG_ERROR("Missing 'CreatePlugin' function in plugin library '{0}'", name);
			throw std::runtime_error("Initialize plugin");
		}
	}

	Ref<home::DeviceScript> PluginManager::CreateDeviceScript(uint32_t scriptID)
	{
		boost::shared_lock_guard lock(mutex);

		boost::unordered::unordered_map<uint32_t, home::DeviceScriptDescription>::iterator it = deviceScriptList.find(scriptID);
		if (it == deviceScriptList.end())
			return nullptr;

		home::DeviceScriptDescription &desc = (*it).second;

		if (desc.createFunction == nullptr)
			return nullptr;

		return desc.createFunction();
	}
	void PluginManager::RegisterDeviceScript(uint32_t scriptID, home::DeviceScriptDescription &scriptDescription)
	{
		boost::lock_guard lock(mutex);

		if (deviceScriptList.count(scriptID))
			LOG_FATAL("Device script ID already exists");

		deviceScriptList[scriptID] = scriptDescription;
	}

	Ref<home::DeviceManagerScript> PluginManager::CreateDeviceManagerScript(uint32_t scriptID)
	{
		boost::shared_lock_guard lock(mutex);

		boost::unordered::unordered_map<uint32_t, home::DeviceManagerScriptDescription>::iterator it = deviceManagerScriptList.find(scriptID);
		if (it == deviceManagerScriptList.end())
			return nullptr;

		home::DeviceManagerScriptDescription &desc = (*it).second;

		if (desc.createFunction == nullptr)
			return nullptr;

		return desc.createFunction();
	}
	void PluginManager::RegisterDeviceManagerScript(uint32_t scriptID, home::DeviceManagerScriptDescription &scriptDescription)
	{
		boost::lock_guard lock(mutex);

		if (deviceManagerScriptList.count(scriptID))
			LOG_FATAL("Device manager script ID already exists");

		deviceManagerScriptList[scriptID] = scriptDescription;
	}

	// Timestamp
	void PluginManager::UpdateTimestamp()
	{
		time_t ts = time(nullptr);
		timestamp = ts;
	}
}