#pragma once
#include "../common.hpp"
#include <utils/Property.hpp>
#include <utils/Timer.hpp>
#include <utils/Event.hpp>
#include "../scripting/Script.hpp"

namespace server
{
	class Home;
	class Room;

	class Script;

	class Database;

	class JsonApi;

	class Action : public boost::enable_shared_from_this<Action>
	{
	private:
		friend class Home;
		friend class Room;
		friend class Database;
		friend class JsonApi;

		//! Timer
		class Timer : public home::Timer
		{
		private:
			friend class Action;

			Ref<Action> action;
			size_t interval;
			boost::asio::deadline_timer timer;
			std::string callback;

			void TimerCallback(const boost::system::error_code& err);

		public:
			Timer(Ref<Action> action, const std::string& callback);
			~Timer();

			virtual void Start(size_t interval) override;

			virtual void Invoke() override;

			virtual void Stop() override;
		};

		//! Event
		class Event : public home::Event
		{
		private:
			friend class Action;

			Ref<Action> action;
			std::string callback;

		public:
			Event(Ref<Action> action, const std::string& callback);
			~Event();

			virtual void Invoke() override;
		};

		boost::shared_mutex mutex;

		std::string name;
		const identifier_t actionID;
		Ref<Room> room;

		//! Interface: Attributes, Properties, Events, Timers
		boost::mutex interfaceMutex;
		robin_hood::unordered_node_map<std::string, rapidjson::Document> attributeList;
		robin_hood::unordered_node_map<std::string, Ref<home::Property>> propertyList;
		robin_hood::unordered_node_map<std::string, Ref<Event>> eventList;
		robin_hood::unordered_node_map<std::string, Ref<Timer>> timerList;

		bool AddAttribute(const std::string& id, const char* json);
		bool RemoveAttribute(const std::string& id);

		Ref<home::Property> AddProperty(const std::string& id, Ref<home::Property> property);
		bool RemoveProperty(const std::string& id);

		Ref<home::Event> AddEvent(const std::string& id, const std::string& callback);
		bool RemoveEvent(const std::string& id);

		Ref<home::Timer> AddTimer(const std::string& id, const std::string& callback);
		bool RemoveTimer(const std::string& id);

		// Cache for properties
		rapidjson::Document snapshot;

		// Script
		const Ref<Script> script;

	public:
		Action(const std::string& name, identifier_t actionID, Ref<Script> script, Ref<Room> room);
		~Action();
		static Ref<Action> Create(const std::string& name, identifier_t actionID, Ref<Script> script, Ref<Room> room);

		/// @brief Get action name
		/// @return Action name
		std::string GetName();

		/// @brief Set action name
		/// @param v New Action name
		bool SetName(const std::string& v);

		inline identifier_t GetActionID() { return actionID; }

		/// @brief Get room
		/// @return Room (can be null)
		Ref<Room> GetRoom();

		/// @brief Set room
		/// @param v Room (can be null)
		/// @return Successfulness
		bool SetRoom(Ref<Room> v);

		/// @brief Get device plugin id
		/// @return Device plugin id
		inline identifier_t GetScriptSourceID() { return script->GetSourceID(); }

		/// @brief Invoke script event
		/// @param event Event name
		void Invoke(const std::string& event);

		/// @brief Take property snapshot
		void TakeSnapshot();
	};
}