#pragma once
#include "../common.hpp"
#include <home/DeviceController.hpp>
#include <home/DeviceControllerPlugin.hpp>

namespace server
{
	class Home;
	class Room;
	class Device;

	class Database;

	class JsonApi;

	class DeviceController : public home::DeviceController, public boost::enable_shared_from_this<DeviceController>
	{
	private:
		friend class Home;
		friend class Room;
		friend class Device;
		friend class Database;
		friend class JsonApi;

		//! Timer
		class Timer : public home::Timer
		{
		private:
			friend class DeviceController;

			Ref<DeviceController> controller;
			size_t interval;
			boost::asio::deadline_timer timer;
			home::TimerCallback<home::DeviceController>* callback;

			void TimerCallback(const boost::system::error_code& err);

		public:
			Timer(Ref<DeviceController> controller, home::TimerCallback<home::DeviceController>* callback);
			~Timer();

			virtual void Start(size_t interval) override;

			virtual void Invoke() override;

			virtual void Stop() override;
		};

		//! Event
		class Event : public home::Event
		{
		private:
			friend class DeviceController;

			Ref<DeviceController> controller;
			home::EventCallback<home::DeviceController>* callback;

		public:
			Event(Ref<DeviceController> controller, home::EventCallback<home::DeviceController>* callback);
			~Event();

			virtual void Invoke() override;
		};

		boost::shared_mutex mutex;

		std::string name;
		const identifier_t controllerID;
		Ref<Room> room;

		//! Interface: Attributes, Properties, Events, Timers
		boost::mutex interfaceMutex;
		robin_hood::unordered_node_map<std::string, rapidjson::Document> attributeList;
		robin_hood::unordered_node_map<std::string, Ref<home::Property>> propertyList;
		robin_hood::unordered_node_map<std::string, Ref<Event>> eventList;
		robin_hood::unordered_node_map<std::string, Ref<Timer>> timerList;

		virtual bool AddAttribute(const std::string& id, const char* json) override;
		virtual bool RemoveAttribute(const std::string& id) override;

		virtual Ref<home::Property> AddProperty(const std::string& id, Ref<home::Property> property) override;
		virtual bool RemoveProperty(const std::string& id) override;

		virtual Ref<home::Event> AddEvent(const std::string& id, home::EventCallback<home::DeviceController>* callback) override;
		virtual bool RemoveEvent(const std::string& id) override;

		virtual Ref<home::Timer> AddTimer(const std::string& id, home::TimerCallback<home::DeviceController>* callback) override;
		virtual bool RemoveTimer(const std::string& id) override;

		// Cacke for properties
		rapidjson::Document snapshot;

		 // Plugin
		const Ref<home::DeviceControllerPlugin> plugin;

		virtual Ref<home::DeviceControllerPlugin> GetPlugin() override { return plugin; }

	public:
		DeviceController(const std::string& name, identifier_t controllerID, Ref<home::DeviceControllerPlugin> plugin, Ref<Room> room);
		~DeviceController();
		static Ref<DeviceController> Create(const std::string& name, identifier_t controllerID, Ref<home::DeviceControllerPlugin> plugin, Ref<Room> room);

		/// @brief Get device controller name
		/// @return Device name
		std::string GetName();

		/// @brief Set device controller name
		/// @param v New device name
		/// @return Successfulness
		bool SetName(const std::string& v);

		inline identifier_t GetDeviceControllerID() { return controllerID; }

		/// @brief Get room
		/// @return Room (can be null)
		Ref<Room> GetRoom();

		/// @brief Set room
		/// @param v Room (can be null)
		/// @return Successfulness
		bool SetRoom(Ref<Room> v);

		/// @brief Get device controller plugin id
		/// @return Device controller plugin id
		inline identifier_t GetPluginID() { return plugin->GetPluginID(); }

		/// @brief Initialize plugin
		void Initialize();

		/// @brief Invoke plugin event
		/// @param event Eveent name
		void TakeSnapshot();

		void Invoke(const std::string& event);

		/// @brief Teraminate plugin
		void Terminate();
	};
}