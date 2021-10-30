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
	void JsonApi::BuildJsonAckMessageWS(rapidjson::Document& output)
	{
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		output.AddMember("msg", rapidjson::Value("ack", 3, allocator), allocator);
	}
	void JsonApi::BuildJsonNAckMessageWS(rapidjson::Document& output)
	{
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		output.AddMember("msg", rapidjson::Value("nack", 4, allocator), allocator);
	}

	void JsonApi::ProcessJsonGetTimestampsMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		Ref<PluginManager> pluginManager = PluginManager::GetInstance();
		assert(pluginManager != nullptr);

		//Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
		//assert(scriptManager != nullptr);

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		Ref<UserManager> userManager = UserManager::GetInstance();
		assert(userManager != nullptr);

		output.AddMember("plugins", rapidjson::Value(pluginManager->timestamp), allocator);
		output.AddMember("scripts", rapidjson::Value(/*scriptManager->timestamp*/0xff), allocator);
		output.AddMember("home", rapidjson::Value(home->timestamp), allocator);
		output.AddMember("users", rapidjson::Value(userManager->timestamp), allocator);

		BuildJsonAckMessageWS(output);
	}

	// Settings
	void JsonApi::ProcessJsonGetSettingsMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		BuildJsonSettings(user, output, allocator);

		BuildJsonAckMessageWS(output);
	}
	void JsonApi::ProcessJsonSetSettingsMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(user != nullptr);
		assert(input.IsObject() && output.IsObject());

		if (user->accessLevel != UserAccessLevel::kAdministratorUserAccessLevel)
		{
			context.Error("Invalid access level. User needs to be administrator.");
			BuildJsonNAckMessageWS(output);
			return;
		}

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

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

		BuildJsonAckMessageWS(output);
	}
}