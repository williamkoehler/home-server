#include "JsonApi.h"
#include "../Core.h"

#include "../scripting/DraftManager.h"
#include "../scripting/Field.h"
#include "../scripting/Draft.h"

namespace server
{
	// Scripting
	void JsonApi::BuildJsonDraftSources(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(output.IsObject());

		Ref<scripting::DraftManager> scriptManager = scripting::DraftManager::GetInstance();
		assert(scriptManager != nullptr);

		boost::shared_lock_guard lock(scriptManager->mutex);

		output.AddMember("timestamp", rapidjson::Value(scriptManager->timestamp), allocator);

		// DraftSources
		rapidjson::Value draftSourceListJson = rapidjson::Value(rapidjson::kArrayType);

		boost::unordered::unordered_map<uint32_t, Ref<scripting::DraftSource>>& draftSourceList = scriptManager->draftSourceList;
		for (std::pair<uint32_t, Ref<scripting::DraftSource>> item : draftSourceList)
		{
			rapidjson::Value draftSourceJson = rapidjson::Value(rapidjson::kObjectType);

			BuildJsonDraftSource(item.second, draftSourceJson, allocator);

			draftSourceListJson.PushBack(draftSourceJson, allocator);
		}

		output.AddMember("draftsources", draftSourceListJson, allocator);
	}

	void JsonApi::BuildJsonDraftSource(const Ref<scripting::DraftSource>& source, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(source != nullptr);
		assert(output.IsObject());

		boost::lock_guard lock(source->mutex);

		output.AddMember("name", rapidjson::Value(source->name.c_str(), source->name.size()), allocator);
		output.AddMember("id", rapidjson::Value(source->sourceID), allocator);

		std::string lang = scripting::DraftLanguageToString(source->language);
		output.AddMember("language", rapidjson::Value(lang.c_str(), lang.size(), allocator), allocator);
	}
	void JsonApi::DecodeJsonDraftSource(const Ref<scripting::DraftSource>& source, rapidjson::Value& input)
	{
		assert(source != nullptr);
		assert(input.IsObject());

		// Decode draft properties if they exist
		{
			boost::lock_guard lock(source->mutex);

			rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
			if (nameIt != input.MemberEnd() && nameIt->value.IsString())
				source->name.assign(nameIt->value.GetString(), nameIt->value.GetStringLength());
		}
	}

	void JsonApi::BuildJsonDraftSourceContent(const Ref<scripting::DraftSource>& source, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(source != nullptr);
		assert(output.IsObject());

		boost::lock_guard lock(source->mutex);

		output.AddMember("content", rapidjson::Value((const char*)source->data, source->length, allocator), allocator);
	}
	void JsonApi::DecodeJsonDraftSourceContent(const Ref<scripting::DraftSource>& source, rapidjson::Value& input)
	{
		assert(source != nullptr);
		assert(input.IsObject());

		// Decode draft properties if they exist
		{
			boost::lock_guard lock(source->mutex);

			rapidjson::Value::MemberIterator sourceIt = input.FindMember("content");
			if (sourceIt != input.MemberEnd() && sourceIt->value.IsString())
				source->Update((uint8_t*)sourceIt->value.GetString(), sourceIt->value.GetStringLength());
		}
	}
	
	void JsonApi::BuildJsonDraft(const Ref<scripting::Draft>& draft, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(draft != nullptr);
		assert(output.IsObject());

		// Lock mutex
		boost::lock_guard lock(draft->mutex);

		rapidjson::Value propertyListJson(rapidjson::kArrayType);

		boost::unordered::unordered_map<identifier_t, Ref<scripting::Field>>& fieldList = draft->fieldList;

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
	void JsonApi::BuildJsonDraftState(const Ref<scripting::Draft>& draft, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(draft != nullptr);
		assert(output.IsObject());

		// Lock mutex
		boost::lock_guard lock(draft->mutex);

		rapidjson::Value fieldListJson(rapidjson::kArrayType);

		boost::unordered::unordered_map<identifier_t, Ref<scripting::Field>>& fieldList = draft->fieldList;

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
	void JsonApi::DecodeJsonDraftState(const Ref<scripting::Draft>& draft, rapidjson::Value& input)
	{
		assert(draft != nullptr);
		assert(input.IsObject());

		// Lock mutex
		boost::lock_guard lock(draft->mutex);

		// Decode draft properties if they exist
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
							boost::unordered::unordered_map<identifier_t, Ref<scripting::Field>>::iterator it = draft->fieldList.find(idIt->value.GetUint64());
							if (it != draft->fieldList.end())
								it->second->Set(valueIt->value);
						}
					}
				}
			}
		}
	}
}