#include "Action.hpp"
#include "../scripting/ScriptManager.hpp"
#include "../scripting/Script.hpp"

namespace server
{
	Action::Action(std::string name, uint32_t actionID, Ref<scripting::Script> script)
		: roomCount(0), name(std::move(name)), actionID(actionID), script(std::move(script))
	{ }
	Action::~Action()
	{ }
	Ref<Action> Action::Create(std::string name, uint32_t actionID, uint32_t scriptSourceID)
	{
		assert(actionID != 0);
		assert(scriptSourceID != 0);

		Ref<scripting::Script> script = scripting::Script::Create(scriptSourceID);
		if (script == nullptr)
		{
			LOG_ERROR("Create script from source '{0}'", scriptSourceID);
			return nullptr;
		}

		// Create new actiom
		Ref<Action> action = boost::make_shared<Action>(name, actionID, std::move(script));
		if (action == nullptr)
			return nullptr;

		return action;
	}

	bool Action::Run()
	{
		boost::shared_lock_guard lock(mutex);

		if (script != nullptr)
		{
			// Check for state
			if (!script->IsReady())
			{
				if (!script->Compile())
					return false;
			}

			// Run script
			return script->Run();
		}
		else
			return false;
	}

	void Action::Load(rapidjson::Value& json)
	{
		assert(json.IsObject());

		boost::lock_guard lock(mutex);
	}
	void Action::Save(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator)
	{
		assert(json.IsObject());

		boost::shared_lock_guard lock(mutex);

		json.AddMember("name", rapidjson::Value(name.c_str(), name.size(), allocator), allocator);
		json.AddMember("id", rapidjson::Value(actionID), allocator);
		json.AddMember("script-source-id", rapidjson::Value(script->GetSource()->GetSourceID()), allocator);
	}
}