#include "script_manager.hpp"
#include <api/user.hpp>
#include <api/websocket_session.hpp>

namespace server
{
    namespace scripting
    {
        void ScriptManager::WebSocketProcessGetScriptSourcesMessage(const Ref<api::User>& user,
                                                                    const api::ApiRequestMessage& request,
                                                                    api::ApiResponseMessage& response,
                                                                    const Ref<api::WebSocketSession>& session)
        {
            (void)user;
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

        void ScriptManager::WebSocketProcessAddScriptSourceMessage(const Ref<api::User>& user,
                                                                   const api::ApiRequestMessage& request,
                                                                   api::ApiResponseMessage& response,
                                                                   const Ref<api::WebSocketSession>& session)
        {
            (void)session;

            if (user->GetAccessLevel() < api::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
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
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            scripting::ScriptLanguage language =
                scripting::ParseScriptLanguage(std::string(langIt->value.GetString(), langIt->value.GetStringLength()));
            if (language == scripting::ScriptLanguage::kUnknownScriptLanguage)
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidArguments);
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
                    response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InternalError);
                    return;
                }

                scriptSource->JsonGet(output, allocator);
            }
        }
        void ScriptManager::WebSocketProcessRemoveScriptSourceMessage(const Ref<api::User>& user,
                                                                      const api::ApiRequestMessage& request,
                                                                      api::ApiResponseMessage& response,
                                                                      const Ref<api::WebSocketSession>& session)
        {
            (void)session;

            if (user->GetAccessLevel() < api::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator sourceIdIt = input.FindMember("id");
            if (sourceIdIt == input.MemberEnd() || !sourceIdIt->value.IsUint())
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
                assert(scriptManager != nullptr);

                if (!scriptManager->RemoveScriptSource(sourceIdIt->value.GetUint()))
                {
                    //! Error failed to remove script source
                    response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InternalError);
                    return;
                }
            }
        }

        void ScriptManager::WebSocketProcessGetScriptSourceMessage(const Ref<api::User>& user,
                                                                   const api::ApiRequestMessage& request,
                                                                   api::ApiResponseMessage& response,
                                                                   const Ref<api::WebSocketSession>& session)
        {
            (void)session;

            if (user->GetAccessLevel() < api::UserAccessLevel::kNormalUserAccessLevel)
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator sourceIdIt = input.FindMember("id");
            if (sourceIdIt == input.MemberEnd() || !sourceIdIt->value.IsUint())
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
                assert(scriptManager != nullptr);

                Ref<scripting::ScriptSource> scriptSource = scriptManager->GetScriptSource(sourceIdIt->value.GetUint());
                if (scriptSource == nullptr)
                {
                    response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                scriptSource->JsonGet(output, allocator);
            }
        }
        void ScriptManager::WebSocketProcessSetScriptSourceMessage(const Ref<api::User>& user,
                                                                   const api::ApiRequestMessage& request,
                                                                   api::ApiResponseMessage& response,
                                                                   const Ref<api::WebSocketSession>& session)
        {
            (void)session;

            if (user->GetAccessLevel() < api::UserAccessLevel::kNormalUserAccessLevel)
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator sourceIdIt = input.FindMember("id");
            if (sourceIdIt == input.MemberEnd() || !sourceIdIt->value.IsUint())
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
                assert(scriptManager != nullptr);

                Ref<scripting::ScriptSource> scriptSource = scriptManager->GetScriptSource(sourceIdIt->value.GetUint());
                if (scriptSource == nullptr)
                {
                    response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                if (scriptSource->JsonSet(input))
                    scriptSource->Save();

                scriptSource->JsonGet(output, allocator);
            }
        }

        void ScriptManager::WebSocketProcessGetScriptSourceContentMessage(const Ref<api::User>& user,
                                                                          const api::ApiRequestMessage& request,
                                                                          api::ApiResponseMessage& response,
                                                                          const Ref<api::WebSocketSession>& session)
        {
            if (user->GetAccessLevel() < api::UserAccessLevel::kNormalUserAccessLevel)
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator sourceIdIt = input.FindMember("id");
            if (sourceIdIt == input.MemberEnd() || !sourceIdIt->value.IsUint())
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
                assert(scriptManager != nullptr);

                Ref<scripting::ScriptSource> scriptSource = scriptManager->GetScriptSource(sourceIdIt->value.GetUint());
                if (scriptSource == nullptr)
                {
                    response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                scriptSource->JsonGetContent(output, allocator);
            }
        }
        void ScriptManager::WebSocketProcessSetScriptSourceContentMessage(const Ref<api::User>& user,
                                                                          const api::ApiRequestMessage& request,
                                                                          api::ApiResponseMessage& response,
                                                                          const Ref<api::WebSocketSession>& session)
        {
            (void)session;

            if (user->GetAccessLevel() < api::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }
            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator sourceIdIt = input.FindMember("id");
            if (sourceIdIt == input.MemberEnd() || !sourceIdIt->value.IsUint())
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
                assert(scriptManager != nullptr);

                Ref<scripting::ScriptSource> scriptSource = scriptManager->GetScriptSource(sourceIdIt->value.GetUint());
                if (scriptSource == nullptr)
                {
                    response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                if (scriptSource->JsonSetContent(input))
                    scriptSource->SaveContent();
            }
        }
    }
}