#pragma once
#include "../common.h"
#include "Home.h"
#include <home/Action.h>
#include "../scripting/Draft.h"

namespace server
{
	class Action : public home::Action, public boost::enable_shared_from_this<Action>
	{
	private:
		friend class Home;
		friend class Room;
		friend class JsonApi;

		boost::shared_mutex mutex;

		size_t roomCount;

		std::string name;
		const uint32_t actionID;

		const Ref<scripting::Draft> draft;

	public:
		Action(std::string name, uint32_t actionID, Ref<scripting::Draft> draft);
		~Action();
		static Ref<Action> Create(std::string name, uint32_t actionID, uint32_t draftSourceID);

		virtual inline Ref<home::Home> GetHome() override { return Home::GetInstance(); }

		virtual inline std::string GetName() override
		{
			boost::shared_lock lock(mutex);
			return name;
		}
		virtual inline void SetName(std::string v) override
		{
			boost::lock_guard lock(mutex);
			name = v;
		}

		virtual inline uint32_t GetActionID() override { return actionID; }

		inline Ref<scripting::Draft> GetDraft() { return draft; }
		inline uint32_t GetDraftSourceID() { return draft->GetSourceID(); }

		scripting::ExecutionResult Reset();
		scripting::ExecutionResult Execute();

		//IO
		void Load(rapidjson::Value& json);
		void Save(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator);
	};
}