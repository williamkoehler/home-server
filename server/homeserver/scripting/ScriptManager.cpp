#include "ScriptManager.hpp"
#include "../database/Database.hpp"
#include "javascript/JSScript.hpp"

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
		if (scriptManager == nullptr)
			return nullptr;

		try
		{
			Ref<Database> database = Database::GetInstance();
			assert(database != nullptr);

			// Load rooms
			database->LoadScriptSources(
				boost::bind(&ScriptManager::LoadScriptSource, scriptManager,
					boost::placeholders::_1,
					boost::placeholders::_2,
					boost::placeholders::_3,
					boost::placeholders::_4,
					boost::placeholders::_5));
		}
		catch (std::exception)
		{
			return nullptr;
		}

		return scriptManager;
	}
	Ref<ScriptManager> ScriptManager::GetInstance()
	{
		return Ref<ScriptManager>(instanceScriptManager);
	}

	//! ScriptSource
	bool ScriptManager::LoadScriptSource(identifier_t sourceID, const std::string& name, ScriptUsage usage, ScriptLanguage language, const std::string_view& data)
	{
		Ref<ScriptSource> source = boost::make_shared<ScriptSource>(name, sourceID, usage, language, data);

		if (source != nullptr)
		{
			scriptSourceList[sourceID] = source;

			return true;
		}
		else
			return false;
	}
	Ref<ScriptSource> ScriptManager::AddScriptSource(const std::string& name, ScriptUsage usage, ScriptLanguage language)
	{
		boost::lock_guard lock(mutex);

		Ref<Database> database = Database::GetInstance();
		assert(database != nullptr);

		// Reserve script source in database
		identifier_t sourceID = database->ReserveScriptSource();
		if (sourceID == 0)
			return nullptr;

		// Create new device
		const char* empty =
			R"(// This script is empty!)"
			R"(// Please replace this with proper code.)";
		Ref<ScriptSource> source = boost::make_shared<ScriptSource>(name, sourceID, usage, language, std::string_view(empty));
		if (source == nullptr)
			return nullptr;

		if (source != nullptr)
		{
			if (!database->UpdateScriptSource(source))
				return nullptr;

			scriptSourceList[sourceID] = source;
		}
		else
		{
			database->RemoveScriptSource(sourceID);
			return nullptr;
		}

		//UpdateTimestamp();

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

		if (scriptSourceList.erase(sourceID))
		{
			Ref<Database> database = Database::GetInstance();
			assert(database != nullptr);

			database->RemoveScriptSource(sourceID);

			return true;
		}
		else
			return false;
	}

	//! Script
	Ref<Script> ScriptManager::CreateActionScript(identifier_t sourceID)
	{
		Ref<ScriptSource> source = GetScriptSource(sourceID);
		if (source != nullptr)
		{
			switch (source->GetLanguage())
			{
			case ScriptLanguage::kJSScriptLanguage:
				return boost::make_shared<javascript::JSScript>(source);
			default:
				return nullptr;
			}
		}

		return nullptr;
	}
}