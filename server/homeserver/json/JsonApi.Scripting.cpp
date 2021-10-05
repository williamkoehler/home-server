#include "JsonApi.hpp"
#include "../Core.hpp"

#include "../scripting/ScriptManager.hpp"
#include "../scripting/Field.hpp"
#include "../scripting/Script.hpp"

namespace server
{
	// Scripting
	void JsonApi::BuildJsonScriptSources(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(output.IsObject());

		Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
		assert(scriptManager != nullptr);

		boost::shared_lock_guard lock(scriptManager->mutex);

		output.AddMember("timestamp", rapidjson::Value(scriptManager->timestamp), allocator);

		// ScriptSources
		rapidjson::Value ScriptSourceListJson = rapidjson::Value(rapidjson::kArrayType);

		boost::unordered::unordered_map<uint32_t, Ref<scripting::ScriptSource>>& ScriptSourceList = scriptManager->scriptSourceList;
		for (std::pair<uint32_t, Ref<scripting::ScriptSource>> item : ScriptSourceList)
		{
			rapidjson::Value ScriptSourceJson = rapidjson::Value(rapidjson::kObjectType);

			BuildJsonScriptSource(item.second, ScriptSourceJson, allocator);

			ScriptSourceListJson.PushBack(ScriptSourceJson, allocator);
		}

		output.AddMember("ScriptSources", ScriptSourceListJson, allocator);
	}

	void JsonApi::BuildJsonScriptSource(const Ref<scripting::ScriptSource>& source, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(source != nullptr);
		assert(output.IsObject());

		boost::lock_guard lock(source->mutex);

		output.AddMember("name", rapidjson::Value(source->name.c_str(), source->name.size()), allocator);
		output.AddMember("id", rapidjson::Value(source->sourceID), allocator);

		std::string lang = scripting::ScriptLanguageToString(source->language);
		output.AddMember("language", rapidjson::Value(lang.c_str(), lang.size(), allocator), allocator);
	}
	void JsonApi::DecodeJsonScriptSource(const Ref<scripting::ScriptSource>& source, rapidjson::Value& input)
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

	void JsonApi::BuildJsonScriptSourceContent(const Ref<scripting::ScriptSource>& source, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(source != nullptr);
		assert(output.IsObject());

		boost::lock_guard lock(source->mutex);

		output.AddMember("content", rapidjson::Value((const char*)source->data, source->length, allocator), allocator);
	}
	void JsonApi::DecodeJsonScriptSourceContent(const Ref<scripting::ScriptSource>& source, rapidjson::Value& input)
	{
		assert(source != nullptr);
		assert(input.IsObject());

		// Decode script properties if they exist
		{
			boost::lock_guard lock(source->mutex);

			rapidjson::Value::MemberIterator sourceIt = input.FindMember("content");
			if (sourceIt != input.MemberEnd() && sourceIt->value.IsString())
				source->Update((uint8_t*)sourceIt->value.GetString(), sourceIt->value.GetStringLength());
		}
	}
	
	void JsonApi::BuildJsonScript(const Ref<scripting::Script>& script, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(script != nullptr);
		assert(output.IsObject());

		// Lock mutex
		boost::lock_guard lock(script->mutex);

		rapidjson::Value propertyListJson(rapidjson::kArrayType);

		boost::unordered::unordered_map<identifier_t, Ref<scripting::Field>>& fieldList = script->fieldList;

		// Reserve memory
		propertyListJson.Reserve(fieldList.size(), allocator);

		for (std::pair<identifier_t, Ref<scripting::Field>> it : fieldList)
		{
			Ref<scripting::Field> field = it.second;

			rapidjson::Value propertyJson(rapidjson::kObjectType);

			// Get representation
			field->GetRepresentation(propertyJson, allocator);

			propertyListJson.PushBack(propertyJson, allocator);
		}

		output.AddMember("properties", propertyListJson, allocator);
	}
	void JsonApi::BuildJsonScriptState(const Ref<scripting::Script>& script, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(script != nullptr);
		assert(output.IsObject());

		// Lock mutex
		boost::lock_guard lock(script->mutex);

		rapidjson::Value fieldListJson(rapidjson::kArrayType);

		boost::unordered::unordered_map<identifier_t, Ref<scripting::Field>>& fieldList = script->fieldList;

		// Reserve memory
		fieldListJson.Reserve(fieldList.size(), allocator);

		for (std::pair<identifier_t, Ref<scripting::Field>> it : fieldList)
		{
			Ref<scripting::Field> field = it.second;

			rapidjson::Value fieldJson(rapidjson::kObjectType);

			// Get representation
			field->GetRepresentation(fieldJson, allocator);

			fieldListJson.PushBack(fieldJson, allocator);
		}

		output.AddMember("fields", fieldListJson, allocator);
	}
	void JsonApi::DecodeJsonScriptState(const Ref<scripting::Script>& script, rapidjson::Value& input)
	{
		assert(script != nullptr);
		assert(input.IsObject());

		// Lock mutex
		boost::lock_guard lock(script->mutex);

		// Decode script properties if they exist
		{
			rapidjson::Value::MemberIterator fieldsIt = input.FindMember("fields");
			if (fieldsIt != input.MemberEnd() && fieldsIt->value.IsArray())
			{
				for (rapidjson::Value::ValueIterator it = fieldsIt->value.Begin(); it != fieldsIt->value.End(); it++)
				{
					if (it->IsObject())
					{
						rapidjson::Value::MemberIterator idIt = it->FindMember("id");
						rapidjson::Value::MemberIterator valueIt = it->FindMember("value");

						if (idIt != it->MemberEnd() && idIt->value.IsUint64() &&
							valueIt != it->MemberEnd())
						{
							boost::unordered::unordered_map<identifier_t, Ref<scripting::Field>>::iterator it = script->fieldList.find(idIt->value.GetUint64());
							if (it != script->fieldList.end())
								it->second->Set(valueIt->value);
						}
					}
				}
			}
		}
	}
}