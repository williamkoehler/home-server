#include "Action.h"
#include "../scripting/DraftManager.h"
#include "../scripting/Draft.h"

namespace server
{
	Action::Action(std::string name, uint32_t actionID, Ref<scripting::Draft> draft)
		: roomCount(0), name(std::move(name)), actionID(actionID), draft(std::move(draft))
	{ }
	Action::~Action()
	{ }
	Ref<Action> Action::Create(std::string name, uint32_t actionID, uint32_t draftSourceID)
	{
		assert(actionID != 0);
		assert(draftSourceID != 0);

		Ref<scripting::DraftManager> draftManager = scripting::DraftManager::GetInstance();
		assert(draftManager != nullptr);

		Ref<scripting::DraftSource> draftSource = draftManager->GetSource(draftSourceID);
		if (draftSource == nullptr)
		{
			LOG_ERROR("Find draft source '{0}'", draftSourceID);
			return nullptr;
		}

		Ref<scripting::Draft> draft = scripting::Draft::Create(draftSource);
		if (draft == nullptr)
		{
			LOG_ERROR("Create draft from source '{0}'", draftSourceID);
			return nullptr;
		}

		draft->MakeReady();

		//script->Execute();

		Ref<Action> action = boost::make_shared<Action>(name, actionID, draft);
		if (action == nullptr)
			return nullptr;

		return action;
	}

	scripting::ExecutionResult Action::Reset()
	{
		draft->Reset();

		// Make script ready
		return draft->MakeReady();
	}
	scripting::ExecutionResult Action::Execute()
	{
		// Check for state
		if (draft->GetState() != scripting::DraftStates::kInitialized)
		{
			scripting::ExecutionResult result = draft->MakeReady();

			if (!result.IsSuccess())
				return result;
		}

		// Execute script
		return draft->Execute();
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
		json.AddMember("draft-source-id", rapidjson::Value(draft->GetSourceID()), allocator);
	}
}