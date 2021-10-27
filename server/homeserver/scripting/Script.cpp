#include "Script.hpp"
#include "ScriptManager.hpp"
#include "javascript/Script.hpp"

namespace server
{
	Script::Script(const std::string& name, identifier_t scriptID, const Ref<ScriptSource>& source)
		: name(name), scriptID(scriptID), source(source)
	{
	}
	Script::~Script()
	{
	}
	Ref<Script> Script::Create(const std::string& name, identifier_t scriptID, identifier_t sourceID)
	{
		Ref<ScriptManager> scriptManager = ScriptManager::GetInstance();
		assert(scriptManager != nullptr);

		Ref<ScriptSource> source = scriptManager->GetScriptSource(sourceID);
		if (source != nullptr)
		{
			switch (source->GetLanguage())
			{
				case ScriptLanguage::kJSScriptLanguage:
					return javascript::Script::Create(name, scriptID, source);
			}
		}

		return nullptr;
	}
}