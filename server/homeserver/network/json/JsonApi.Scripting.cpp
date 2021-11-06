#include "JsonApi.hpp"
#include "../../Core.hpp"

#include "../../scripting/ScriptManager.hpp"
#include "../../scripting/Script.hpp"

namespace server
{
	// Scripting
	void JsonApi::BuildJsonScriptSources(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(output.IsObject());

		Ref<ScriptManager> scriptManager = ScriptManager::GetInstance();
		assert(scriptManager != nullptr);

		boost::shared_lock_guard lock(scriptManager->mutex);

		// ScriptSources
		rapidjson::Value ScriptSourceListJson = rapidjson::Value(rapidjson::kArrayType);

		boost::unordered::unordered_map<identifier_t, Ref<ScriptSource>>& scriptSourceList = scriptManager->scriptSourceList;
		for (std::pair<identifier_t, Ref<ScriptSource>> item : scriptSourceList)
		{
			rapidjson::Value scriptSourceJson = rapidjson::Value(rapidjson::kObjectType);

			BuildJsonScriptSource(item.second, scriptSourceJson, allocator);

			ScriptSourceListJson.PushBack(scriptSourceJson, allocator);
		}

		output.AddMember("scriptsources", ScriptSourceListJson, allocator);
	}

	void JsonApi::BuildJsonScriptSource(Ref<ScriptSource> source, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(source != nullptr);
		assert(output.IsObject());

		boost::lock_guard lock(source->mutex);

		output.AddMember("name", rapidjson::Value(source->name.c_str(), source->name.size()), allocator);
		output.AddMember("id", rapidjson::Value(source->sourceID), allocator);

		std::string lang = StringifyScriptLanguage(source->language);
		output.AddMember("language", rapidjson::Value(lang.c_str(), lang.size(), allocator), allocator);

		std::string usage = StringifyScriptUsage(source->usage);
		output.AddMember("usage", rapidjson::Value(usage.c_str(), usage.size(), allocator), allocator);
	}
	void JsonApi::DecodeJsonScriptSource(Ref<ScriptSource> source, rapidjson::Value& input)
	{
		assert(source != nullptr);
		assert(input.IsObject());

		// Decode script properties if they exist
		{
			boost::lock_guard lock(source->mutex);

			rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
			if (nameIt != input.MemberEnd() && nameIt->value.IsString())
				source->name.assign(nameIt->value.GetString(), nameIt->value.GetStringLength());
		}
	}

	void JsonApi::BuildJsonScriptSourceData(Ref<ScriptSource> source, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(source != nullptr);
		assert(output.IsObject());

		boost::lock_guard lock(source->mutex);

		output.AddMember("data", rapidjson::Value((const char*)source->data.data(), source->data.size(), allocator), allocator);
	}
	void JsonApi::DecodeJsonScriptSourceData(Ref<ScriptSource> source, rapidjson::Value& input)
	{
		assert(source != nullptr);
		assert(input.IsObject());

		// Decode script properties if they exist
		{
			boost::lock_guard lock(source->mutex);

			rapidjson::Value::MemberIterator sourceIt = input.FindMember("data");
			if (sourceIt != input.MemberEnd() && sourceIt->value.IsString())
				source->SetData(std::string_view((const char*)sourceIt->value.GetString(), sourceIt->value.GetStringLength()));
		}
	}
}