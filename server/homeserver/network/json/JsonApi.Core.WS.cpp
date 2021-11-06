#include "JsonApi.hpp"
#include "../../Core.hpp"
#include "../../tools/EMail.hpp"

#include "../../plugin/PluginManager.hpp"
#include "../../home/Home.hpp"
#include "../../scripting/ScriptManager.hpp"
#include "../../user/UserManager.hpp"
#include "../../user/User.hpp"

namespace server
{
	// Settings
	void JsonApi::ProcessJsonGetSettingsMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		BuildJsonSettings(user, output, allocator);
	}
	void JsonApi::ProcessJsonSetSettingsMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel < UserAccessLevel::kMaintainerUserAccessLevel)
		{
			context.Error(ApiError::kError_AccessLevelToLow);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		// Read core
		rapidjson::Value::MemberIterator coreIt = input.FindMember("core");
		if (coreIt != input.MemberEnd() && coreIt->value.IsObject())
		{
			Ref<Core> core = Core::GetInstance();

			assert(core != nullptr);

			boost::lock_guard lock(core->mutex);

			rapidjson::Value::MemberIterator nameIt = coreIt->value.FindMember("name");
			if (nameIt != coreIt->value.MemberEnd() && nameIt->value.IsString())
				core->name.assign(nameIt->value.GetString(), nameIt->value.GetStringLength());
		}

		// Read emails
		rapidjson::Value::MemberIterator emailIt = input.FindMember("email");
		if (emailIt != input.MemberEnd() && emailIt->value.IsObject())
		{
			Ref<EMail> email = EMail::GetInstance();

			assert(email != nullptr);

			boost::lock_guard lock(email->mutex);

			rapidjson::Value::MemberIterator recipientListIt = emailIt->value.FindMember("recipients");
			if (recipientListIt != emailIt->value.MemberEnd() && recipientListIt->value.IsArray())
			{
				email->recipients.clear();

				for (rapidjson::Value::ValueIterator recipientIt = recipientListIt->value.Begin(); recipientIt != recipientListIt->value.End(); recipientIt++)
				{
					if (recipientIt->IsString())
						email->recipients.push_back(std::string(recipientIt->GetString(), recipientIt->GetStringLength()));
				}
			}
		}
	}
}