#include "JsonApi.hpp"
#include <home-scripting/Script.hpp>
#include <home-scripting/ScriptManager.hpp>
#include <home-scripting/ScriptSource.hpp>

namespace server
{
    namespace networking
    {
        void JsonApi::ProcessJsonGetScriptSourcesMessageWS(const Ref<users::User>& user,
                                                           const ApiRequestMessage& request,
                                                           ApiResponseMessage& response, const Ref<ApiSession>& session)
        {
            (void)session;

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Build response
            {
                Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
                assert(scriptManager != nullptr);

                scriptManager->JsonGet(output, allocator);
            }
        }

        void JsonApi::ProcessJsonAddScriptSourceMessageWS(const Ref<users::User>& user,
                                                          const ApiRequestMessage& request,
                                                          ApiResponseMessage& response, const Ref<ApiSession>& session)
        {
            (void)session;

            if (user->GetAccessLevel() < users::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator nameIt = input.FindMember("name");
            rapidjson::Value::ConstMemberIterator langIt = input.FindMember("language");
            if (nameIt == input.MemberEnd() || !nameIt->value.IsString() || langIt == input.MemberEnd() ||
                !langIt->value.IsString())
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            scripting::ScriptLanguage language =
                scripting::ParseScriptLanguage(std::string(langIt->value.GetString(), langIt->value.GetStringLength()));
            if (language == scripting::ScriptLanguage::kUnknownScriptLanguage)
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
                assert(scriptManager != nullptr);

                rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);
                Ref<scripting::ScriptSource> scriptSource = scriptManager->AddScriptSource(
                    language, std::string(nameIt->value.GetString(), nameIt->value.GetStringLength()));
                if (scriptSource == nullptr)
                {
                    //! Error failed to add script source
                    response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InternalError);
                    return;
                }

                scriptSource->JsonGet(output, allocator);
            }
        }
        void JsonApi::ProcessJsonRemoveScriptSourceMessageWS(const Ref<users::User>& user,
                                                             const ApiRequestMessage& request,
                                                             ApiResponseMessage& response,
                                                             const Ref<ApiSession>& session)
        {
            (void)session;

            if (user->GetAccessLevel() < users::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator sourceIdIt = input.FindMember("id");
            if (sourceIdIt == input.MemberEnd() || !sourceIdIt->value.IsUint())
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
                assert(scriptManager != nullptr);

                if (!scriptManager->RemoveScriptSource(sourceIdIt->value.GetUint()))
                {
                    //! Error failed to remove script source
                    response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InternalError);
                    return;
                }
            }
        }

        void JsonApi::ProcessJsonGetScriptSourceMessageWS(const Ref<users::User>& user,
                                                          const ApiRequestMessage& request,
                                                          ApiResponseMessage& response, const Ref<ApiSession>& session)
        {
            (void)session;

            if (user->GetAccessLevel() < users::UserAccessLevel::kNormalUserAccessLevel)
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator sourceIdIt = input.FindMember("id");
            if (sourceIdIt == input.MemberEnd() || !sourceIdIt->value.IsUint())
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
                assert(scriptManager != nullptr);

                Ref<scripting::ScriptSource> scriptSource = scriptManager->GetScriptSource(sourceIdIt->value.GetUint());
                if (scriptSource == nullptr)
                {
                    response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                scriptSource->JsonGet(output, allocator);
            }
        }
        void JsonApi::ProcessJsonSetScriptSourceMessageWS(const Ref<users::User>& user,
                                                          const ApiRequestMessage& request,
                                                          ApiResponseMessage& response, const Ref<ApiSession>& session)
        {
            (void)session;

            if (user->GetAccessLevel() < users::UserAccessLevel::kNormalUserAccessLevel)
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator sourceIdIt = input.FindMember("id");
            if (sourceIdIt == input.MemberEnd() || !sourceIdIt->value.IsUint())
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
                assert(scriptManager != nullptr);

                Ref<scripting::ScriptSource> scriptSource = scriptManager->GetScriptSource(sourceIdIt->value.GetUint());
                if (scriptSource == nullptr)
                {
                    response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                if (scriptSource->JsonSet(input))
                    scriptSource->Save();

                scriptSource->JsonGet(output, allocator);
            }
        }

        void JsonApi::ProcessJsonGetScriptSourceContentMessageWS(const Ref<users::User>& user,
                                                                 const ApiRequestMessage& request,
                                                                 ApiResponseMessage& response,
                                                                 const Ref<ApiSession>& session)
        {
            if (user->GetAccessLevel() < users::UserAccessLevel::kNormalUserAccessLevel)
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator sourceIdIt = input.FindMember("id");
            if (sourceIdIt == input.MemberEnd() || !sourceIdIt->value.IsUint())
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
                assert(scriptManager != nullptr);

                Ref<scripting::ScriptSource> scriptSource = scriptManager->GetScriptSource(sourceIdIt->value.GetUint());
                if (scriptSource == nullptr)
                {
                    response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                scriptSource->JsonGetContent(output, allocator);
            }
        }
        void JsonApi::ProcessJsonSetScriptSourceContentMessageWS(const Ref<users::User>& user,
                                                                 const ApiRequestMessage& request,
                                                                 ApiResponseMessage& response,
                                                                 const Ref<ApiSession>& session)
        {
            (void)session;

            if (user->GetAccessLevel() < users::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }
            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator sourceIdIt = input.FindMember("id");
            if (sourceIdIt == input.MemberEnd() || !sourceIdIt->value.IsUint())
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
                assert(scriptManager != nullptr);

                Ref<scripting::ScriptSource> scriptSource = scriptManager->GetScriptSource(sourceIdIt->value.GetUint());
                if (scriptSource == nullptr)
                {
                    response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                if (scriptSource->JsonSetContent(input))
                    scriptSource->SaveContent();
            }
        }
    }
}