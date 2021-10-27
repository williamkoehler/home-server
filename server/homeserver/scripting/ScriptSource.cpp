#include "ScriptSource.hpp"

namespace server
{
	ScriptSource::ScriptSource(const std::string& name, identifier_t sourceID, ScriptUsage usage, ScriptLanguage language)
		: name(name), sourceID(sourceID), usage(usage), language(language)
	{ }
	ScriptSource::~ScriptSource()
	{
	}

	Ref<ScriptSource> ScriptSource::Create(const std::string& name, identifier_t sourceID, ScriptUsage usage, ScriptLanguage language)
	{
		Ref<ScriptSource> scriptSource = boost::make_shared<ScriptSource>(name, sourceID, usage, language);

		return scriptSource;
	}
}
