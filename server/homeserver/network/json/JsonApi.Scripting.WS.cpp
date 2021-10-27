//#include "JsonApi.hpp"
//#include "../Core.hpp"
//
//#include "../scripting/ScriptManager.hpp"
//#include "../scripting/ScriptSource.hpp"
//#include "../scripting/Script.hpp"
//
//#include "../user/UserManager.hpp"
//#include "../user/User.hpp"
//
//namespace server
//{
//	void JsonApi::ProcessJsonGetScriptSourcesMessageWS(rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
//	{
//		assert(input.IsObject() && output.IsObject());
//
//		// Build response
//		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();
//
//		BuildJsonScriptSources(output, allocator);
//
//		BuildJsonAckMessageWS(output);
//	}
//
//	void JsonApi::ProcessJsonAddScriptSourceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
//	{
//		assert(user != nullptr);
//		assert(input.IsObject() && output.IsObject());
//
//		if (user->accessLevel != UserAccessLevels::kAdministratorUserAccessLevel)
//		{
//			context.Error("Invalid access level. User needs to be administrator.");
//			BuildJsonNAckMessageWS(output);
//			return;
//		}
//
//		// Process request
//		rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
//		rapidjson::Value::MemberIterator langIt = input.FindMember("language");
//		if (nameIt == input.MemberEnd() || !nameIt->value.IsString() ||
//			langIt == input.MemberEnd() || !langIt->value.IsString())
//		{
//			context.Error("Missing name and/or language");
//			BuildJsonNAckMessageWS(output);
//			return;
//		}
//
//		scripting::ScriptLanguage language = scripting::ScriptLanguageFromString(std::string(langIt->value.GetString(), langIt->value.GetStringLength()));
//		if (language == scripting::ScriptLanguage::kUnknownScriptLanguage)
//		{
//			context.Error("Invalid script language");
//			BuildJsonNAckMessageWS(output);
//			return;
//		}
//
//		// Build response
//		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();
//
//		Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
//		assert(scriptManager != nullptr);
//
//		rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);
//		Ref<scripting::ScriptSource> source = scriptManager->AddSource(nameIt->value.GetString(), 0, language, nullptr, 0);
//		if (source == nullptr)
//		{
//			context.Error("Add source");
//			BuildJsonNAckMessageWS(output);
//			return;
//		}
//
//		BuildJsonScriptSource(source, output, allocator);
//
//		BuildJsonAckMessageWS(output);
//	}
//	void JsonApi::ProcessJsonRemoveScriptSourceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
//	{
//		assert(user != nullptr);
//		assert(input.IsObject() && output.IsObject());
//
//		if (user->accessLevel != UserAccessLevels::kAdministratorUserAccessLevel)
//		{
//			context.Error("Invalid access level. User needs to be administrator.");
//			BuildJsonNAckMessageWS(output);
//			return;
//		}
//
//		// Process request
//		rapidjson::Value::MemberIterator sourceIDIt = input.FindMember("id");
//		if (sourceIDIt == input.MemberEnd() || !sourceIDIt->value.IsUint())
//		{
//			context.Error("Missing id");
//			BuildJsonNAckMessageWS(output);
//			return;
//		}
//
//		// Build response
//		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();
//
//		Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
//		assert(scriptManager != nullptr);
//
//		try
//		{
//			scriptManager->RemoveSource(sourceIDIt->value.GetUint());
//		}
//		catch (std::exception)
//		{
//			context.Error("Invalid id");
//			BuildJsonNAckMessageWS(output);
//			return;
//		}
//
//		BuildJsonAckMessageWS(output);
//	}
//
//	void JsonApi::ProcessJsonGetScriptSourceMessageWS(rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
//	{
//		assert(input.IsObject() && output.IsObject());
//
//		// Process request
//		rapidjson::Value::MemberIterator sourceIDIt = input.FindMember("id");
//		if (sourceIDIt == input.MemberEnd() || !sourceIDIt->value.IsUint())
//		{
//			context.Error("Missing id");
//			BuildJsonNAckMessageWS(output);
//			return;
//		}
//
//		// Build response
//		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();
//
//		Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
//		assert(scriptManager != nullptr);
//
//		Ref<scripting::ScriptSource> source = scriptManager->GetSource(sourceIDIt->value.GetUint());
//		if (source == nullptr)
//		{
//			context.Error("Invalid id");
//			BuildJsonNAckMessageWS(output);
//			return;
//		}
//
//		// Build script source
//		BuildJsonScriptSource(source, output, allocator);
//		
//		BuildJsonAckMessageWS(output);
//	}
//	void JsonApi::ProcessJsonSetScriptSourceMessageWS(rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
//	{
//		assert(input.IsObject() && output.IsObject());
//
//		// Process request
//		rapidjson::Value::MemberIterator sourceIDIt = input.FindMember("id");
//		if (sourceIDIt == input.MemberEnd() || !sourceIDIt->value.IsUint())
//		{
//			context.Error("Missing id");
//			BuildJsonNAckMessageWS(output);
//			return;
//		}
//
//		// Build response
//		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();
//
//		Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
//		assert(scriptManager != nullptr);
//
//		Ref<scripting::ScriptSource> source = scriptManager->GetSource(sourceIDIt->value.GetUint());
//		if (source == nullptr)
//		{
//			context.Error("Invalid id");
//			BuildJsonNAckMessageWS(output);
//			return;
//		}
//
//		// Decode script source
//		DecodeJsonScriptSource(source, input);
//
//		BuildJsonAckMessageWS(output);
//	}
//
//	void JsonApi::ProcessJsonGetScriptSourceContentMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
//	{
//		assert(user != nullptr);
//		assert(input.IsObject() && output.IsObject());
//
//		if (user->accessLevel != UserAccessLevels::kAdministratorUserAccessLevel)
//		{
//			context.Error("Invalid access level. User needs to be administrator.");
//			BuildJsonNAckMessageWS(output);
//			return;
//		}
//
//		// Process request
//		rapidjson::Value::MemberIterator sourceIDIt = input.FindMember("id");
//		if (sourceIDIt == input.MemberEnd() || !sourceIDIt->value.IsUint())
//		{
//			context.Error("Missing id");
//			BuildJsonNAckMessageWS(output);
//			return;
//		}
//
//		// Build response
//		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();
//
//		Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
//		assert(scriptManager != nullptr);
//
//		Ref<scripting::ScriptSource> source = scriptManager->GetSource(sourceIDIt->value.GetUint());
//		if (source == nullptr)
//		{
//			context.Error("Invalid id");
//			BuildJsonNAckMessageWS(output);
//			return;
//		}
//
//		// Build script source
//		BuildJsonScriptSourceContent(source, output, allocator);
//
//		BuildJsonAckMessageWS(output);
//	}
//	void JsonApi::ProcessJsonSetScriptSourceContentMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
//	{
//		assert(user != nullptr);
//		assert(input.IsObject() && output.IsObject());
//
//		if (user->accessLevel != UserAccessLevels::kAdministratorUserAccessLevel)
//		{
//			context.Error("Invalid access level. User needs to be administrator.");
//			BuildJsonNAckMessageWS(output);
//			return;
//		}
//
//		// Process request
//		rapidjson::Value::MemberIterator sourceIDIt = input.FindMember("id");
//		if (sourceIDIt == input.MemberEnd() || !sourceIDIt->value.IsUint())
//		{
//			context.Error("Missing id");
//			BuildJsonNAckMessageWS(output);
//			return;
//		}
//
//		// Build response
//		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();
//
//		Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
//		assert(scriptManager != nullptr);
//
//		Ref<scripting::ScriptSource> source = scriptManager->GetSource(sourceIDIt->value.GetUint());
//		if (source == nullptr)
//		{
//			context.Error("Invalid id");
//			BuildJsonNAckMessageWS(output);
//			return;
//		}
//
//		// Decode script source
//		DecodeJsonScriptSourceContent(source, input);
//
//		BuildJsonAckMessageWS(output);
//	}
//}