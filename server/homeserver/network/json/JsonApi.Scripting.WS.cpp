#include "JsonApi.hpp"
#include "../../scripting/ScriptManager.hpp"
#include "../../scripting/ScriptSource.hpp"
#include "../../scripting/Script.hpp"

#include "../../user/UserManager.hpp"
#include "../../user/User.hpp"

namespace server
{
	void JsonApi::ProcessJsonGetScriptSourcesMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		BuildJsonScriptSources(output, allocator);
	}

	void JsonApi::ProcessJsonAddScriptSourceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel < UserAccessLevel::kMaintainerUserAccessLevel)
		{
			context.Error(ApiError::kError_AccessLevelToLow);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
		rapidjson::Value::MemberIterator usageIt = input.FindMember("usage");
		rapidjson::Value::MemberIterator langIt = input.FindMember("language");
		if (nameIt == input.MemberEnd() || !nameIt->value.IsString() ||
			usageIt == input.MemberEnd() || !usageIt->value.IsString() ||
			langIt == input.MemberEnd() || !langIt->value.IsString())
		{
			context.Error("Missing name, usage and/or language");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		ScriptUsage usage = ParseScriptUsage(std::string(usageIt->value.GetString(), usageIt->value.GetStringLength()));
		if (usage == ScriptUsage::kUnknownUsage)
		{
			context.Error("Invalid script usage");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		ScriptLanguage language = ParseScriptLanguage(std::string(langIt->value.GetString(), langIt->value.GetStringLength()));
		if (language == ScriptLanguage::kUnknownScriptLanguage)
		{
			context.Error("Invalid script language");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<ScriptManager> scriptManager = ScriptManager::GetInstance();
		assert(scriptManager != nullptr);

		rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);
		Ref<ScriptSource> source = scriptManager->AddScriptSource(nameIt->value.GetString(), usage, language);
		if (source == nullptr)
		{
			//! Error failed to add script source
			context.Error(ApiError::kError_InternalError);
			return;
		}

		BuildJsonScriptSource(source, output, allocator);
	}
	void JsonApi::ProcessJsonRemoveScriptSourceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel < UserAccessLevel::kMaintainerUserAccessLevel)
		{
			context.Error(ApiError::kError_AccessLevelToLow);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator sourceIDIt = input.FindMember("id");
		if (sourceIDIt == input.MemberEnd() || !sourceIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<ScriptManager> scriptManager = ScriptManager::GetInstance();
		assert(scriptManager != nullptr);

		if (!scriptManager->RemoveScriptSource(sourceIDIt->value.GetUint()))
		{
			//! Error failed to remove script source
			context.Error(ApiError::kError_InternalError);
			return;
		}
	}

	void JsonApi::ProcessJsonGetScriptSourceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel < UserAccessLevel::kNormalUserAccessLevel)
		{
			context.Error(ApiError::kError_AccessLevelToLow);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator sourceIDIt = input.FindMember("id");
		if (sourceIDIt == input.MemberEnd() || !sourceIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<ScriptManager> scriptManager = ScriptManager::GetInstance();
		assert(scriptManager != nullptr);

		Ref<ScriptSource> source = scriptManager->GetScriptSource(sourceIDIt->value.GetUint());
		if (source == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Build script source
		BuildJsonScriptSource(source, output, allocator);
	}
	void JsonApi::ProcessJsonSetScriptSourceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel < UserAccessLevel::kNormalUserAccessLevel)
		{
			context.Error(ApiError::kError_AccessLevelToLow);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator sourceIDIt = input.FindMember("id");
		if (sourceIDIt == input.MemberEnd() || !sourceIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<ScriptManager> scriptManager = ScriptManager::GetInstance();
		assert(scriptManager != nullptr);

		Ref<ScriptSource> source = scriptManager->GetScriptSource(sourceIDIt->value.GetUint());
		if (source == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Decode script source
		DecodeJsonScriptSource(source, input);
	}

	void JsonApi::ProcessJsonGetScriptSourceDataMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel < UserAccessLevel::kNormalUserAccessLevel)
		{
			context.Error(ApiError::kError_AccessLevelToLow);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator sourceIDIt = input.FindMember("id");
		if (sourceIDIt == input.MemberEnd() || !sourceIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<ScriptManager> scriptManager = ScriptManager::GetInstance();
		assert(scriptManager != nullptr);

		Ref<ScriptSource> source = scriptManager->GetScriptSource(sourceIDIt->value.GetUint());
		if (source == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Build script source
		BuildJsonScriptSourceData(source, output, allocator);
	}
	void JsonApi::ProcessJsonSetScriptSourceDataMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel < UserAccessLevel::kMaintainerUserAccessLevel)
		{
			context.Error(ApiError::kError_AccessLevelToLow);
			return;
		}

		// Process request
		rapidjson::Value::MemberIterator sourceIDIt = input.FindMember("id");
		if (sourceIDIt == input.MemberEnd() || !sourceIDIt->value.IsUint())
		{
			context.Error("Missing id");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<ScriptManager> scriptManager = ScriptManager::GetInstance();
		assert(scriptManager != nullptr);

		Ref<ScriptSource> source = scriptManager->GetScriptSource(sourceIDIt->value.GetUint());
		if (source == nullptr)
		{
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Decode script source
		DecodeJsonScriptSourceData(source, input);
	}
}