#include "ScriptManager.hpp"
#include "ScriptSource.hpp"

namespace scripting
{
	ScriptLanguage ScriptLanguageFromString(std::string lang)
	{
		const char* c = lang.c_str();

		if (strncmp(c, "javascript", 10) == 0)
			return ScriptLanguage::kJSScriptLanguage;
		else
			return ScriptLanguage::kUnknownScriptLanguage;
	}
	std::string ScriptLanguageToString(ScriptLanguage language)
	{
		switch (language)
		{
			case ScriptLanguage::kJSScriptLanguage:
				return "javascript";
			default:
				return "unknown";
		}
	}

	std::string ScriptExtFromScriptLanguage(ScriptLanguage language)
	{
		switch (language)
		{
			case ScriptLanguage::kJSScriptLanguage:
				return ".js";
			default:
				return ".unknown";
		}
	}

	boost::weak_ptr<ScriptManager> instanceScriptManager;

	ScriptManager::ScriptManager()
	{
	}
	ScriptManager::~ScriptManager()
	{
	}

	Ref<ScriptManager> ScriptManager::Create()
	{
		if (!instanceScriptManager.expired())
			return Ref<ScriptManager>(instanceScriptManager);

		Ref<ScriptManager> scriptManager = boost::make_shared<ScriptManager>();
		instanceScriptManager = scriptManager;

		LOG_INFO("Initializing script manager");

		try
		{
			// // Initilize lua engine
			// scriptManager->luaEngine = lua::LuaEngine::Create();
			// if (scriptManager->luaEngine == nullptr)
			// 	return nullptr;

			// Load
			scriptManager->Load();
		}
		catch (std::exception e)
		{
			LOG_EXCEPTION("{0}", e.what());
		}

		return scriptManager;
	}
	Ref<ScriptManager> ScriptManager::GetInstance()
	{
		return Ref<ScriptManager>(instanceScriptManager);
	}

	//Timestamp
	void ScriptManager::UpdateTimestamp()
	{
		const time_t ts = time(nullptr);
		timestamp = ts;
	}

	Ref<ScriptSource> ScriptManager::AddSource(std::string name, uint32_t sourceID, ScriptLanguage language, uint8_t* data, size_t length)
	{
		uint32_t genID = sourceID ? sourceID : XXH32(name.c_str(), name.size(), 0x41435343);

		// Check for duplicate
		{
			boost::shared_lock_guard lock(mutex);

			size_t pass = 10;
			while (scriptSourceList.count(genID) || !genID)
			{
				genID++;

				//Only allow 10 passes
				if (!(pass--))
				{
					LOG_ERROR("Generate unique id for script source '{0}'", name);
					return nullptr;
				}
			}
		}

		Ref<ScriptSource> script = ScriptSource::Create(name, sourceID, language);
		if (script == nullptr)
			return nullptr;

		if (data != nullptr && length > 0)
			script->Update(data, length);

		boost::lock_guard lock(mutex);
		scriptSourceList[genID] = script;

		UpdateTimestamp();

		return script;
	}

	Ref<ScriptSource> ScriptManager::GetSource(uint32_t sourceID)
	{
		boost::shared_lock_guard lock(mutex);

		boost::unordered::unordered_map<uint32_t, Ref<ScriptSource>>::iterator it = scriptSourceList.find(sourceID);
		if (it == scriptSourceList.end())
			return nullptr;

		return (*it).second;
	}

	void ScriptManager::RemoveSource(uint32_t sourceID)
	{
		boost::lock_guard lock(mutex);

		boost::unordered::unordered_map<uint32_t, Ref<ScriptSource>>::iterator it = scriptSourceList.find(sourceID);
		if (it == scriptSourceList.end())
			throw std::runtime_error("Source ID does not exist");

		scriptSourceList.erase(it);

		UpdateTimestamp();
	}
}