#include "ScriptManager.hpp"

namespace server
{
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

		if (scriptManager != nullptr)
		{

		}

		return scriptManager;
	}

	Ref<ScriptManager> ScriptManager::GetInstance()
	{
		return Ref<ScriptManager>(instanceScriptManager);
	}

	Ref<ScriptSource> ScriptManager::AddScriptSource(const std::string& name, identifier_t sourceID, ScriptUsage usage, ScriptLanguage language)
	{
		boost::lock_guard lock(mutex);

		identifier_t genID = sourceID ? sourceID : XXH32(name.c_str(), name.size(), 0x53535243);

		// Check for duplicate
		{
			size_t pass = 10;
			while (scriptSourceList.count(genID))
			{
				genID++;

				//Only allow 10 passes
				if (!(pass--))
				{
					LOG_ERROR("Failing to generate unique id for script source '{0}'", name);
					return nullptr;
				}
			}
		}

		Ref<ScriptSource> source = ScriptSource::Create(name, genID, usage, language);
		if (source != nullptr)
			scriptSourceList[genID] = source;

		return source;
	}
	Ref<ScriptSource> ScriptManager::GetScriptSource(identifier_t sourceID)
	{
		boost::shared_lock_guard lock(mutex);

		const boost::unordered::unordered_map<identifier_t, Ref<ScriptSource>>::const_iterator it = scriptSourceList.find(sourceID);
		if (it == scriptSourceList.end())
			return nullptr;

		return (*it).second;
	}
	bool ScriptManager::RemoveScriptSource(identifier_t sourceID)
	{
		boost::lock_guard lock(mutex);
		return scriptSourceList.erase(sourceID);
	}
}