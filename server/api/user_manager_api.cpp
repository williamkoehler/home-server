#include "user_manager.hpp"

namespace server
{
    namespace api
    {
        void UserManager::WebSocketProcessGetUsersMessage(const Ref<api::User>& user, const ApiRequestMessage& request,
                                                   ApiResponseMessage& response, const Ref<WebSocketSession>& session)
        {
            (void)session;
            
            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Build response
            {
                Ref<api::UserManager> userManager = api::UserManager::GetInstance();
                assert(userManager != nullptr);

                userManager->JsonGet(output, allocator);
            }
        }

        void UserManager::WebSocketProcessAddUserMessage(const Ref<api::User>& user, const ApiRequestMessage& request,
                                                  ApiResponseMessage& response, const Ref<WebSocketSession>& session)
        {
            (void)session;

            if (user->GetAccessLevel() < UserAccessLevel::kMaintainerUserAccessLevel)
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator nameIt = input.FindMember("name");
            rapidjson::Value::ConstMemberIterator accessLevelIt = input.FindMember("accesslevel");
            if (nameIt == input.MemberEnd() || !nameIt->value.IsString() || accessLevelIt == input.MemberEnd() ||
                !accessLevelIt->value.IsString())
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<api::UserManager> userManager = api::UserManager::GetInstance();
                assert(userManager != nullptr);

                rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);
                Ref<api::User> user2 =
                    userManager->AddUser(nameIt->value.GetString(), nameIt->value.GetString(),
                                         api::ParseUserAccessLevel(accessLevelIt->value.GetString()));
                if (user2 == nullptr)
                {
                    //! Error failed to add user
                    response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InternalError);
                    return;
                }

                user2->JsonGet(output, allocator);
            }
        }
        void UserManager::WebSocketProcessRemoveUserMessage(const Ref<api::User>& user, const ApiRequestMessage& request,
                                                     ApiResponseMessage& response, const Ref<WebSocketSession>& session)
        {
            (void)session;

            if (user->GetAccessLevel() < api::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator userIdIt = input.FindMember("id");
            if (userIdIt == input.MemberEnd() || !userIdIt->value.IsUint())
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<api::UserManager> userManager = api::UserManager::GetInstance();
                assert(userManager != nullptr);

                if (!userManager->RemoveUser(userIdIt->value.GetUint()))
                {
                    //! Error failed to remove user
                    response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InternalError);
                    return;
                }
            }
        }

        void UserManager::WebSocketProcessGetUserMessage(const Ref<api::User>& user, const ApiRequestMessage& request,
                                                  ApiResponseMessage& response, const Ref<WebSocketSession>& session)
        {
            (void)session;

            if (user->GetAccessLevel() < api::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator userIDIt = input.FindMember("id");
            if (userIDIt == input.MemberEnd() || !userIDIt->value.IsUint())
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<api::UserManager> userManager = api::UserManager::GetInstance();
                assert(userManager != nullptr);

                Ref<api::User> user2 = userManager->GetUser(userIDIt->value.GetUint());
                if (user2 == nullptr)
                {
                    //! Error user does not exist
                    response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                user2->JsonGet(output, allocator);
            }
        }
        void UserManager::WebSocketProcessSetUserMessage(const Ref<api::User>& user, const ApiRequestMessage& request,
                                                  ApiResponseMessage& response, const Ref<WebSocketSession>& session)
        {
            (void)session;

            if (user->GetAccessLevel() < api::UserAccessLevel::kNormalUserAccessLevel)
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator userIdIt = input.FindMember("id");
            if (userIdIt == input.MemberEnd() || !userIdIt->value.IsUint())
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            Ref<api::UserManager> userManager = api::UserManager::GetInstance();
            assert(userManager != nullptr);

            Ref<api::User> user2 = userManager->GetUser(userIdIt->value.GetUint());
            if (user2 == nullptr)
            {
                //! Error user does not exist
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                return;
            }

            // Check if the user wants to change another user, since this
            // can only be done by an admin user
            if (user->GetID() != user2->GetID() &&
                user->GetAccessLevel() < api::UserAccessLevel::kAdministratorUserAccessLevel)
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }

            user2->JsonSet(input);

            rapidjson::Value::ConstMemberIterator newPasswordIt = input.FindMember("newpassword");
            if (newPasswordIt != input.MemberEnd() && newPasswordIt->value.IsString())
            {
                rapidjson::Value::ConstMemberIterator passwordIt = input.FindMember("password");

                if (passwordIt != input.MemberEnd() && passwordIt->value.IsString())
                {
                    Ref<api::UserManager> userManager = api::UserManager::GetInstance();
                    assert(userManager != nullptr);

                    // Assign password
                    if (userManager->SetUserPassword(
                            user, std::string(passwordIt->value.GetString(), passwordIt->value.GetStringLength()),
                            std::string(newPasswordIt->value.GetString(), newPasswordIt->value.GetStringLength())))
                    {
                        //! Error invalid password
                        response.SetErrorCode(ApiErrorCodes::kApiErrorCode_UserInvalidPassword);
                        return;
                    }
                }
            }
        }
    }
}