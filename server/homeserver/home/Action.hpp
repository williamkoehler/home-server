#pragma once
#include "../common.hpp"
#include "Home.hpp"
#include <home/Action.hpp>
#include "../scripting/Script.hpp"

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

		Ref<scripting::Script> script;

	public:
		Action(std::string name, uint32_t actionID, Ref<scripting::Script> script);
		~Action();
		static Ref<Action> Create(std::string name, uint32_t actionID, uint32_t scriptSourceID);

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

		inline Ref<scripting::Script> GetScript()
		{
			boost::shared_lock_guard lock(mutex); 
			return script;
		}
		inline void SetScript(Ref<scripting::Script> v)
		{
			assert(v != nullptr);
			boost::lock_guard lock(mutex);
			script = v;
		}

		inline uint32_t GetScriptSourceID()
		{
			boost::shared_lock_guard lock(mutex);
			if (script != nullptr)
				return script->GetSource()->GetSourceID();
			else
				return 0;
		}

		/// @brief Run action
		/// @return Successfulness
		bool Run();

		/// @brief Load action from JSON
		/// @param json JSON
		void Load(rapidjson::Value& json);

		/// @brief Save action to JSON
		/// @param json JSON
		/// @param allocator JSON allocator 
		void Save(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator);
	};
}