#include "JsonApi.hpp"
#include <home-scripting/Script.hpp>
#include <home-scripting/ScriptManager.hpp>
#include <home-scripting/ScriptSource.hpp>

namespace server
{
    namespace networking
    {
        void JsonApi::ProcessJsonGetScriptSourcesMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                           rapidjson::Document& output, ApiContext& context)
        {
            assert(input.IsObject() && output.IsObject());

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
            assert(scriptManager != nullptr);

            scriptManager->JsonGet(output, allocator);
        }

        void JsonApi::ProcessJsonAddScriptSourceMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                          rapidjson::Document& output, ApiContext& context)
        {
            assert(user != nullptr);
            assert(input.IsObject() && output.IsObject());

            if (user->GetAccessLevel() < users::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                context.Error(ApiError::kError_AccessLevelToLow);
                return;
            }

            // Process request
            rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
            rapidjson::Value::MemberIterator usageIt = input.FindMember("usage");
            rapidjson::Value::MemberIterator langIt = input.FindMember("language");
            if (nameIt == input.MemberEnd() || !nameIt->value.IsString() || usageIt == input.MemberEnd() ||
                !usageIt->value.IsString() || langIt == input.MemberEnd() || !langIt->value.IsString())
            {
                context.Error("Missing name, usage and/or language");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            scripting::ScriptUsage usage =
                scripting::ParseScriptUsage(std::string(usageIt->value.GetString(), usageIt->value.GetStringLength()));
            if (usage == scripting::ScriptUsage::kUnknownUsage)
            {
                context.Error("Invalid script usage");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            scripting::ScriptLanguage language =
                scripting::ParseScriptLanguage(std::string(langIt->value.GetString(), langIt->value.GetStringLength()));
            if (language == scripting::ScriptLanguage::kUnknownScriptLanguage)
            {
                context.Error("Invalid script language");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
            assert(scriptManager != nullptr);

            rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);
            Ref<scripting::ScriptSource> scriptSource = scriptManager->AddScriptSource(
                language, std::string(nameIt->value.GetString(), nameIt->value.GetStringLength()), usage);
            if (scriptSource == nullptr)
            {
                //! Error failed to add script source
                context.Error(ApiError::kError_InternalError);
                return;
            }

            scriptSource->JsonGet(output, allocator);
        }
        void JsonApi::ProcessJsonRemoveScriptSourceMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                             rapidjson::Document& output, ApiContext& context)
        {
            assert(user != nullptr);
            assert(input.IsObject() && output.IsObject());

            if (user->GetAccessLevel() < users::UserAccessLevel::kMaintainerUserAccessLevel)
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

            Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
            assert(scriptManager != nullptr);

            if (!scriptManager->RemoveScriptSource(sourceIDIt->value.GetUint()))
            {
                //! Error failed to remove script source
                context.Error(ApiError::kError_InternalError);
                return;
            }
        }

        void JsonApi::ProcessJsonGetScriptSourceMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                          rapidjson::Document& output, ApiContext& context)
        {
            assert(input.IsObject() && output.IsObject());

            if (user->GetAccessLevel() < users::UserAccessLevel::kNormalUserAccessLevel)
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

            Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
            assert(scriptManager != nullptr);

            Ref<scripting::ScriptSource> scriptSource = scriptManager->GetScriptSource(sourceIDIt->value.GetUint());
            if (scriptSource == nullptr)
            {
                context.Error(ApiError::kError_InvalidIdentifier);
                return;
            }

            // Build script source
            scriptSource->JsonGet(output, allocator);
        }
        void JsonApi::ProcessJsonSetScriptSourceMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                          rapidjson::Document& output, ApiContext& context)
        {
            assert(input.IsObject() && output.IsObject());

            if (user->GetAccessLevel() < users::UserAccessLevel::kNormalUserAccessLevel)
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

            Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
            assert(scriptManager != nullptr);

            Ref<scripting::ScriptSource> scriptSource = scriptManager->GetScriptSource(sourceIDIt->value.GetUint());
            if (scriptSource == nullptr)
            {
                context.Error(ApiError::kError_InvalidIdentifier);
                return;
            }

            // Decode script source
            scriptSource->JsonSet(input);
        }

        void JsonApi::ProcessJsonGetScriptSourceContentMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                              rapidjson::Document& output, ApiContext& context)
        {
            assert(user != nullptr);
            assert(input.IsObject() && output.IsObject());

            if (user->GetAccessLevel() < users::UserAccessLevel::kNormalUserAccessLevel)
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

            Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
            assert(scriptManager != nullptr);

            Ref<scripting::ScriptSource> scriptSource = scriptManager->GetScriptSource(sourceIDIt->value.GetUint());
            if (scriptSource == nullptr)
            {
                context.Error(ApiError::kError_InvalidIdentifier);
                return;
            }

            // Build script source
            scriptSource->JsonGetContent(output, allocator);
        }
        void JsonApi::ProcessJsonSetScriptSourceContentMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                              rapidjson::Document& output, ApiContext& context)
        {
            assert(user != nullptr);
            assert(input.IsObject() && output.IsObject());

            if (user->GetAccessLevel() < users::UserAccessLevel::kMaintainerUserAccessLevel)
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

            Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
            assert(scriptManager != nullptr);

            Ref<scripting::ScriptSource> scriptSource = scriptManager->GetScriptSource(sourceIDIt->value.GetUint());
            if (scriptSource == nullptr)
            {
                context.Error(ApiError::kError_InvalidIdentifier);
                return;
            }

            // Decode script source content
            scriptSource->JsonSetContent(input);
        }
    }
}