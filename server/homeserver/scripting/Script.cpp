#include "Script.hpp"
#include "ScriptSource.hpp"
#include "js/JSScript.hpp"

namespace scripting
{
	Script::Script(Ref<ScriptSource> source)
		: source(std::move(source)), lastResult(ScriptResult::Failure("Nothing happened yet."))
	{ }
	Script::~Script()
	{ }
	Ref<Script> Script::Create(uint32_t scriptSourceID)
	{
		// Get script manager
		Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
		assert(scriptManager != nullptr);

		// Get script source
		Ref<scripting::ScriptSource> source = scriptManager->GetSource(scriptSourceID);
		if (source == nullptr)
		{
			LOG_ERROR("Find script source '{0}'", scriptSourceID);
			return nullptr;
		}

		Ref<Script> script = nullptr;

		switch (source->GetLanguage())
		{
			case ScriptLanguage::kJSScriptLanguage:
				script = boost::make_shared<JSScript>(std::move(source));
			default:
				LOG_ERROR("Invalid script language");
				return nullptr;
		}

		if (script != nullptr)
		{
			// Try compiling
			script->Compile();
		}

		return script;
	}
}