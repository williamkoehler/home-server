#include "JsonApi.hpp"
#include "../Core.hpp"
#include "JsonField.hpp"
#include "../tools/EMail.hpp"

namespace server
{
	// Core
	void JsonApi::BuildJsonSettings(const Ref<User>& user, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(output.IsObject());

		// Core settings
		{
			rapidjson::Value coreJson = rapidjson::Value(rapidjson::kObjectType);

			Ref<Core> core = Core::GetInstance();

			assert(core != nullptr);

			boost::lock_guard lock(core->mutex);

			coreJson.AddMember("name", rapidjson::Value(core->name.c_str(), core->name.size(), allocator), allocator);

			output.AddMember("core", coreJson, allocator);
		}

		// EMail settings
		{
			rapidjson::Value emailJson = rapidjson::Value(rapidjson::kObjectType);

			Ref<EMail> email = EMail::GetInstance();

			assert(email != nullptr);

			boost::lock_guard lock(email->mutex);

			rapidjson::Value recipientListJson = rapidjson::Value(rapidjson::kArrayType);

			for (std::string& recipient : email->recipients)
				recipientListJson.PushBack(rapidjson::Value(recipient.c_str(), recipient.size(), allocator), allocator);

			emailJson.AddMember("recipients", recipientListJson, allocator);

			output.AddMember("email", emailJson, allocator);
		}

		// User settings
		{
			//rapidjson::Value userJson = rapidjson::Value(rapidjson::kObjectType);


		}
	}
}