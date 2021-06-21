#include "JsonApi.h"
#include "../Core.h"

#include "../scripting/DraftManager.h"
#include "../scripting/DraftSource.h"
#include "../scripting/Draft.h"

#include "../user/UserManager.h"
#include "../user/User.h"

namespace server
{
	void JsonApi::ProcessJsonGetDraftSourcesMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		BuildJsonDraftSources(output, allocator);

		BuildJsonAckMessageWS(output);
	}

	void JsonApi::ProcessJsonAddDraftSourceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel != UserAccessLevels::kAdministratorUserAccessLevel)
		{
			context.Error("Invalid access level. User needs to be administrator.");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
		rapidjson::Value::MemberIterator langIt = input.FindMember("language");
		if (nameIt == input.MemberEnd() || !nameIt->value.IsString() ||
			langIt == input.MemberEnd() || !langIt->value.IsString())
		{
			context.Error("Missing name and/or language");
			BuildJsonNAckMessageWS(output);
			return;
		}

		scripting::DraftLanguages language = scripting::DraftLanguageFromString(std::string(langIt->value.GetString(), langIt->value.GetStringLength()));
		if (language == scripting::DraftLanguages::kUnknownDraftLanguage)
		{
			context.Error("Invalid draft language");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<scripting::DraftManager> draftManager = scripting::DraftManager::GetInstance();
		assert(draftManager != nullptr);

		rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);
		Ref<scripting::DraftSource> source = draftManager->AddSource(nameIt->value.GetString(), 0, language, nullptr, 0);
		if (source == nullptr)
		{
			context.Error("Add source");
			BuildJsonNAckMessageWS(output);
			return;
		}

		BuildJsonDraftSource(source, output, allocator);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonRemoveDraftSourceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel != UserAccessLevels::kAdministratorUserAccessLevel)
		{
			context.Error("Invalid access level. User needs to be administrator.");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator sourceIDIt = input.FindMember("id");
		if (sourceIDIt == input.MemberEnd() || !sourceIDIt->value.IsUint())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<scripting::DraftManager> draftManager = scripting::DraftManager::GetInstance();
		assert(draftManager != nullptr);

		try
		{
			draftManager->RemoveSource(sourceIDIt->value.GetUint());
		}
		catch (std::exception)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		BuildJsonAckMessageWS(output);
	}

	void JsonApi::ProcessJsonGetDraftSourceMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator sourceIDIt = input.FindMember("id");
		if (sourceIDIt == input.MemberEnd() || !sourceIDIt->value.IsUint())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<scripting::DraftManager> scriptManager = scripting::DraftManager::GetInstance();
		assert(scriptManager != nullptr);

		Ref<scripting::DraftSource> source = scriptManager->GetSource(sourceIDIt->value.GetUint());
		if (source == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build script source
		BuildJsonDraftSource(source, output, allocator);
		
		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonSetDraftSourceMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator sourceIDIt = input.FindMember("id");
		if (sourceIDIt == input.MemberEnd() || !sourceIDIt->value.IsUint())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<scripting::DraftManager> scriptManager = scripting::DraftManager::GetInstance();
		assert(scriptManager != nullptr);

		Ref<scripting::DraftSource> source = scriptManager->GetSource(sourceIDIt->value.GetUint());
		if (source == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Decode script source
		DecodeJsonDraftSource(source, input);

		BuildJsonAckMessageWS(output);
	}

	void JsonApi::ProcessJsonGetDraftSourceContentMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel != UserAccessLevels::kAdministratorUserAccessLevel)
		{
			context.Error("Invalid access level. User needs to be administrator.");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator sourceIDIt = input.FindMember("id");
		if (sourceIDIt == input.MemberEnd() || !sourceIDIt->value.IsUint())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<scripting::DraftManager> scriptManager = scripting::DraftManager::GetInstance();
		assert(scriptManager != nullptr);

		Ref<scripting::DraftSource> source = scriptManager->GetSource(sourceIDIt->value.GetUint());
		if (source == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build script source
		BuildJsonDraftSourceContent(source, output, allocator);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonSetDraftSourceContentMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel != UserAccessLevels::kAdministratorUserAccessLevel)
		{
			context.Error("Invalid access level. User needs to be administrator.");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator sourceIDIt = input.FindMember("id");
		if (sourceIDIt == input.MemberEnd() || !sourceIDIt->value.IsUint())
		{
			context.Error("Missing id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<scripting::DraftManager> scriptManager = scripting::DraftManager::GetInstance();
		assert(scriptManager != nullptr);

		Ref<scripting::DraftSource> source = scriptManager->GetSource(sourceIDIt->value.GetUint());
		if (source == nullptr)
		{
			context.Error("Invalid id");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Decode script source
		DecodeJsonDraftSourceContent(source, input);

		BuildJsonAckMessageWS(output);
	}
}