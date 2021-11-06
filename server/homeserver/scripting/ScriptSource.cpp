#include "ScriptSource.hpp"
#include "../database/Database.hpp"
#include <xxhash/xxhash.h>

namespace server
{
	std::string StringifyScriptUsage(ScriptUsage usage)
	{
		switch (usage)
		{
		case ScriptUsage::kForActionUsage:
			return "for-action";
		default:
			return "unknown";
		}
	}
	ScriptUsage ParseScriptUsage(const std::string& usage)
	{
		if (usage == "for-action")
			return ScriptUsage::kForActionUsage;
		else
			return ScriptUsage::kUnknownUsage;
	}

	std::string StringifyScriptLanguage(ScriptLanguage language)
	{
		switch (language) 
		{
		case ScriptLanguage::kJSScriptLanguage:
			return "javascript";
		default:
			return "unknown";
		}
	}
	ScriptLanguage ParseScriptLanguage(const std::string& language)
	{
		if (language == "javascript")
			return ScriptLanguage::kJSScriptLanguage;
		else
			return ScriptLanguage::kUnknownScriptLanguage;
	}

	ScriptSource::ScriptSource(const std::string& name, identifier_t sourceID, ScriptUsage usage, ScriptLanguage language, const std::string_view& data)
		: name(name), sourceID(sourceID), usage(usage), language(language), data(data)
	{ }
	ScriptSource::~ScriptSource()
	{
	}

	std::string ScriptSource::GetName()
	{
		boost::shared_lock_guard lock(mutex);
		return name;
	}
	bool ScriptSource::SetName(const std::string& v)
	{
		boost::lock_guard lock(mutex);
		if (Database::GetInstance()->UpdateScriptSourcePropName(shared_from_this(), name, v))
		{
			name = v;
			return true;
		}
		else
			return false;
	}

	std::string ScriptSource::GetData()
	{
		boost::shared_lock_guard lock(mutex);
		return data;
	}
	bool ScriptSource::SetData(const std::string_view& v)
	{
		boost::lock_guard lock(mutex);
		if (Database::GetInstance()->UpdateScriptSourcePropData(shared_from_this(), v))
		{
			data.assign(v);
			checksum = XXH64(v.data(), v.size(), 0x53535243);
			return true;
		}
		else
			return false;
	}
}
