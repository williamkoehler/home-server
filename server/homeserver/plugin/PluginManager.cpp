#include "PluginManager.h"

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

		home::DeviceScriptDescription& desc = (*it).second;

		if (desc.createFunction == nullptr)
			return nullptr;

		return desc.createFunction();
	}
	void PluginManager::RegisterDeviceScript(uint32_t scriptID, home::DeviceScriptDescription& scriptDescription)
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

		home::DeviceManagerScriptDescription& desc = (*it).second;

		if (desc.createFunction == nullptr)
			return nullptr;

		return desc.createFunction();
	}
	void PluginManager::RegisterDeviceManagerScript(uint32_t scriptID, home::DeviceManagerScriptDescription& scriptDescription)
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

	void PluginManager::Load()
	{
		FILE* file = fopen("plugins/plugins-info.json", "r");
		if (file == nullptr)
		{
			LOG_ERROR("Open/find 'plugins/plugins-info.json'");
			//SaveDefault();
			throw std::runtime_error("Open/find file 'plugins/plugins-info.json'");
		}

		char buffer[RAPIDJSON_BUFFER_SIZE_SMALL];
		rapidjson::FileReadStream stream(file, buffer, sizeof(buffer));

		rapidjson::Document document;
		if (document.ParseStream(stream).HasParseError() || !document.IsObject())
		{
			LOG_ERROR("Read 'plugins-info.json'");
			throw std::runtime_error("Read file 'plugins/plugins-info.json'");
		}

		//Load plugin
		rapidjson::Value::MemberIterator pluginListIt = document.FindMember("plugins");
		if (pluginListIt == document.MemberEnd() || !pluginListIt->value.IsArray())
		{
			LOG_ERROR("Missing 'plugins' in 'plugins/plugins-info.json'");
			throw std::runtime_error("Invalid file 'plugins/plugins-info.json'");
		}

		rapidjson::Value& pluginListJson = pluginListIt->value;
		for (rapidjson::Value::ValueIterator pluginIt = pluginListJson.Begin(); pluginIt != pluginListJson.End(); pluginIt++)
		{
			if (!pluginIt->IsString())
			{
				LOG_ERROR("Invalid plugin in 'plugins' in 'plugins/plugins-info.json'");
				continue;
			}

			LoadPlugin(std::string(pluginIt->GetString(), pluginIt->GetStringLength()));
		}

		fclose(file);

		UpdateTimestamp();
	}
}