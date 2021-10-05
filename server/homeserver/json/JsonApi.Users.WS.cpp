#include "JsonApi.hpp"
#include "../Core.hpp"

#include "../user/UserManager.hpp"
#include "../user/User.hpp"

namespace server
{
	// User
	void JsonApi::ProcessJsonGetUsersMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request
		rapidjson::Value::MemberIterator timestampIt = input.FindMember("timestamp");

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		size_t timestamp = (timestampIt != input.MemberEnd() && timestampIt->value.IsUint64()) ? timestampIt->value.GetUint64() : 0;

		BuildJsonUsers(output, allocator, timestamp);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonSetUsersMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Process request

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		DecodeJsonUsers(input);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonSetUserMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject());

		// Decode user properties if they exist
		{
			boost::lock_guard lock(user->mutex);

			rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
			if (nameIt != input.MemberEnd() && nameIt->value.IsString())
				user->name.assign(nameIt->value.GetString(), nameIt->value.GetStringLength());

			rapidjson::Value::MemberIterator newPasswordIt = input.FindMember("newpassword");
			if (newPasswordIt != input.MemberEnd() && newPasswordIt->value.IsString())
			{
				rapidjson::Value::MemberIterator passwordIt = input.FindMember("password");

				if (passwordIt != input.MemberEnd() && passwordIt->value.IsString())
				{
					if (newPasswordIt->value.GetStringLength() < 8)
					{
						LOG_CODE_MISSING("JsonApiWS.cpp");
					}

					Ref<UserManager> userManager = UserManager::GetInstance();
					assert(userManager != nullptr);

					userManager->SetUserPassword(
						user,
						std::string_view(passwordIt->value.GetString(), passwordIt->value.GetStringLength()),
						std::string_view(newPasswordIt->value.GetString(), newPasswordIt->value.GetStringLength()));
				}
			}
		}
	}
}