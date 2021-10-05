#include "JsonApi.hpp"
#include "../Core.hpp"

#include "../user/UserManager.hpp"
#include "../user/User.hpp"

namespace server
{
	// Users
	void JsonApi::BuildJsonUsers(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator, size_t timestamp)
	{
		Ref<UserManager> userManager = UserManager::GetInstance();

		assert(userManager != nullptr);
		assert(output.IsObject());

		boost::shared_lock_guard lock(userManager->mutex);

		output.AddMember("timestamp", rapidjson::Value(userManager->timestamp), allocator);

		// Only send tree if it has changed
		if (userManager->timestamp > timestamp)
		{
			rapidjson::Value userListJson = rapidjson::Value(rapidjson::kArrayType);

			boost::unordered::unordered_map<uint32_t, Ref<User>> userList = userManager->userList;
			for (std::pair<uint32_t, Ref<User>> item : userList)
			{
				rapidjson::Value userJson = rapidjson::Value(rapidjson::kObjectType);

				BuildJsonUser(item.second, userJson, allocator);

				userListJson.PushBack(userJson, allocator);
			}

			output.AddMember("users", userListJson, allocator);
		}
	}
	void JsonApi::DecodeJsonUsers(rapidjson::Value& input)
	{
		assert(input.IsObject());

		// Decode users if it exists
		rapidjson::Value::MemberIterator userListIt = input.FindMember("users");
		if (userListIt != input.MemberEnd() && userListIt->value.IsArray())
		{
			for (rapidjson::Value::ValueIterator userIt = userListIt->value.Begin(); userIt != userListIt->value.End(); userIt++)
			{
				if (userIt->IsObject())
				{
					rapidjson::Value::MemberIterator userIDIt = input.FindMember("userid");
					if (userIDIt == input.MemberEnd() || !userIDIt->value.IsUint())
						return;

					Ref<UserManager> userManager = UserManager::GetInstance();
					assert(userManager != nullptr);

					Ref<User> user = userManager->GetUser(userIDIt->value.GetUint());
					if (user != nullptr)
						DecodeJsonUser(user, *userIt);
				}
			}
		}
	}
	void JsonApi::BuildJsonUser(const Ref<User>& user, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(user != nullptr);
		assert(output.IsObject());

		boost::shared_lock_guard lock(user->mutex);

		output.AddMember("name", rapidjson::Value(user->name.c_str(), user->name.size()), allocator);
		output.AddMember("userid", rapidjson::Value(user->userID), allocator);
		output.AddMember("accesslevel", rapidjson::Value(static_cast<size_t>(user->accessLevel)), allocator);
	}
	void JsonApi::DecodeJsonUser(const Ref<User>& user, rapidjson::Value& input)
	{
		assert(user != nullptr);
		assert(input.IsObject());

		// Decode user properties if they exist
		{
			boost::lock_guard lock(user->mutex);

			rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
			if (nameIt != input.MemberEnd() && nameIt->value.IsString())
				user->name.assign(nameIt->value.GetString(), nameIt->value.GetStringLength());
		}
	}
}