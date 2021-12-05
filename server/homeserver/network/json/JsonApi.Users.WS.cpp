#include "JsonApi.hpp"
#include "../../user/UserManager.hpp"
#include "../../user/User.hpp"

namespace server
{
	// User
	void JsonApi::ProcessJsonGetUsersMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		BuildJsonUsers(output, allocator);
	}

	void JsonApi::ProcessJsonAddUserMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
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
		rapidjson::Value::MemberIterator accessLevelIt = input.FindMember("accesslevel");
		if (nameIt == input.MemberEnd() || !nameIt->value.IsString() ||
			accessLevelIt == input.MemberEnd() || !accessLevelIt->value.IsString())
		{
			context.Error("Missing name and/or accesslevel");
			context.Error(ApiError::kError_InvalidArguments);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<UserManager> userManager = UserManager::GetInstance();
		assert(userManager != nullptr);

		rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);
		Ref<User> user2 = userManager->AddUser(nameIt->value.GetString(), nameIt->value.GetString(), ParseUserAccessLevel(accessLevelIt->value.GetString()));
		if (user2 == nullptr)
		{
			//! Error failed to add user
			context.Error(ApiError::kError_InternalError);
			return;
		}

		BuildJsonUser(user2, output, allocator);
	}
	void JsonApi::ProcessJsonRemoveUserMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel < UserAccessLevel::kMaintainerUserAccessLevel)
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

		Ref<UserManager> userManager = UserManager::GetInstance();
		assert(userManager != nullptr);

		if (!userManager->RemoveUser(userIDIt->value.GetUint()))
		{
			//! Error failed to remove user
			context.Error(ApiError::kError_InternalError);
			return;
		}
	}

	void JsonApi::ProcessJsonGetUserMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel < UserAccessLevel::kMaintainerUserAccessLevel)
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

		Ref<UserManager> userManager = UserManager::GetInstance();
		assert(userManager != nullptr);

		Ref<User> user2 = userManager->GetUser(userIDIt->value.GetUint());
		if (user2 == nullptr)
		{
			//! Error user does not exist
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Build user
		BuildJsonUser(user2, output, allocator);
	}
	void JsonApi::ProcessJsonSetUserMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject());

		if (user->accessLevel < UserAccessLevel::kNormalUserAccessLevel)
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

		Ref<UserManager> userManager = UserManager::GetInstance();
		assert(userManager != nullptr);

		Ref<User> user2 = userManager->GetUser(userIDIt->value.GetUint());
		if (user2 == nullptr)
		{
			//! Error user does not exist
			context.Error(ApiError::kError_InvalidIdentifier);
			return;
		}

		// Check if the user watns to change another user
		// This is can only be done by a admin user
		if (user->userID != user2->userID && user->accessLevel < UserAccessLevel::kAdministratorUserAccessLevel)
		{
			context.Error(ApiError::kError_AccessLevelToLow);
			return;
		}

		// Decode user
		DecodeJsonUser(user, input);

		// Decode user password
		{
			rapidjson::Value::MemberIterator newPasswordIt = input.FindMember("newpassword");
			if (newPasswordIt != input.MemberEnd() && newPasswordIt->value.IsString())
			{
				rapidjson::Value::MemberIterator passwordIt = input.FindMember("password");

				if (passwordIt != input.MemberEnd() && passwordIt->value.IsString())
				{
					if (newPasswordIt->value.GetStringLength() >= 8 &&
						boost::regex_match(std::string(newPasswordIt->value.GetString(),
							newPasswordIt->value.GetStringLength()), boost::regex(R"(^[a-zA-Z0-9!@#$%^&+\-/\*~,;.:-_|<>=]*$)")))
					{
						Ref<UserManager> userManager = UserManager::GetInstance();
						assert(userManager != nullptr);

						// Assign password
						userManager->SetUserPassword(
							user,
							std::string_view(passwordIt->value.GetString(), passwordIt->value.GetStringLength()),
							std::string_view(newPasswordIt->value.GetString(), newPasswordIt->value.GetStringLength()));
					}
					else
					{
						//! Error invalid password
						context.Error("Password must be at least 8 characters long and only contain '" R"(a-zA-Z0-9!@#$%^&+-/\*~,;.:-_|<>=]*$)" "'");
						context.Error(ApiError::kError_UserInvalidPassword);
						return;
					}
				}
			}
		}
	}
}