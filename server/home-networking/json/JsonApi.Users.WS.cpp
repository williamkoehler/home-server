#include "JsonApi.hpp"
#include <home-users/UserManager.hpp>

namespace server
{
    namespace networking
    {
        void JsonApi::ProcessJsonGetUsersMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                   rapidjson::Document& output, ApiContext& context)
        {
            assert(input.IsObject() && output.IsObject());

            Ref<users::UserManager> userManager = users::UserManager::GetInstance();
            assert(userManager != nullptr);

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            userManager->JsonGet(output, allocator);
        }

        void JsonApi::ProcessJsonAddUserMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
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
            rapidjson::Value::MemberIterator accessLevelIt = input.FindMember("accesslevel");
            if (nameIt == input.MemberEnd() || !nameIt->value.IsString() || accessLevelIt == input.MemberEnd() ||
                !accessLevelIt->value.IsString())
            {
                context.Error("Missing name and/or accesslevel");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<users::UserManager> userManager = users::UserManager::GetInstance();
            assert(userManager != nullptr);

            rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);
            Ref<users::User> user2 =
                userManager->AddUser(nameIt->value.GetString(), nameIt->value.GetString(),
                                     users::ParseUserAccessLevel(accessLevelIt->value.GetString()));
            if (user2 == nullptr)
            {
                //! Error failed to add user
                context.Error(ApiError::kError_InternalError);
                return;
            }

            user2->JsonGet(output, allocator);
        }
        void JsonApi::ProcessJsonRemoveUserMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
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
            rapidjson::Value::MemberIterator userIDIt = input.FindMember("id");
            if (userIDIt == input.MemberEnd() || !userIDIt->value.IsUint())
            {
                context.Error("Missing id");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<users::UserManager> userManager = users::UserManager::GetInstance();
            assert(userManager != nullptr);

            if (!userManager->RemoveUser(userIDIt->value.GetUint()))
            {
                //! Error failed to remove user
                context.Error(ApiError::kError_InternalError);
                return;
            }
        }

        void JsonApi::ProcessJsonGetUserMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
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
            rapidjson::Value::MemberIterator userIDIt = input.FindMember("id");
            if (userIDIt == input.MemberEnd() || !userIDIt->value.IsUint())
            {
                context.Error("Missing id");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<users::UserManager> userManager = users::UserManager::GetInstance();
            assert(userManager != nullptr);

            Ref<users::User> user2 = userManager->GetUser(userIDIt->value.GetUint());
            if (user2 == nullptr)
            {
                //! Error user does not exist
                context.Error(ApiError::kError_InvalidIdentifier);
                return;
            }

            // Build user
            user2->JsonGet(output, allocator);
        }
        void JsonApi::ProcessJsonSetUserMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                  rapidjson::Document& output, ApiContext& context)
        {
            assert(user != nullptr);
            assert(input.IsObject());

            if (user->GetAccessLevel() < users::UserAccessLevel::kNormalUserAccessLevel)
            {
                context.Error(ApiError::kError_AccessLevelToLow);
                return;
            }

            // Process request
            rapidjson::Value::MemberIterator userIDIt = input.FindMember("id");
            if (userIDIt == input.MemberEnd() || !userIDIt->value.IsUint())
            {
                context.Error("Missing id");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<users::UserManager> userManager = users::UserManager::GetInstance();
            assert(userManager != nullptr);

            Ref<users::User> user2 = userManager->GetUser(userIDIt->value.GetUint());
            if (user2 == nullptr)
            {
                //! Error user does not exist
                context.Error(ApiError::kError_InvalidIdentifier);
                return;
            }

            // Check if the user wants to change another user, since this
            // can only be done by an admin user
            if (user->GetID() != user2->GetID() && user->GetAccessLevel() < users::UserAccessLevel::kAdministratorUserAccessLevel)
            {
                context.Error(ApiError::kError_AccessLevelToLow);
                return;
            }

            user2->JsonSet(input);

            rapidjson::Value::MemberIterator newPasswordIt = input.FindMember("newpassword");
            if (newPasswordIt != input.MemberEnd() && newPasswordIt->value.IsString())
            {
                rapidjson::Value::MemberIterator passwordIt = input.FindMember("password");

                if (passwordIt != input.MemberEnd() && passwordIt->value.IsString())
                {
                    if (newPasswordIt->value.GetStringLength() >= 8 &&
                        boost::regex_match(
                            std::string(newPasswordIt->value.GetString(), newPasswordIt->value.GetStringLength()),
                            boost::regex(R"(^[a-zA-Z0-9!@#$%^&+\-/\*~,;.:-_|<>=]*$)")))
                    {
                        Ref<users::UserManager> userManager = users::UserManager::GetInstance();
                        assert(userManager != nullptr);

                        // Assign password
                        userManager->SetUserPassword(
                            user, std::string_view(passwordIt->value.GetString(), passwordIt->value.GetStringLength()),
                            std::string_view(newPasswordIt->value.GetString(), newPasswordIt->value.GetStringLength()));
                    }
                    else
                    {
                        //! Error invalid password
                        context.Error("Password must be at least 8 characters long and only contain '"
                                      R"(a-zA-Z0-9!@#$%^&+-/\*~,;.:-_|<>=]*$)"
                                      "'");
                        context.Error(ApiError::kError_UserInvalidPassword);
                        return;
                    }
                }
            }
        }
    }
}