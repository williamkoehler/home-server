#pragma once
#include "../common.hpp"
#include <home/Device.hpp>
#include <home/DevicePlugin.hpp>

namespace server
{
	class Home;
	class Room;
	class DeviceController;

	class Database;

	class JsonApi;

	class Device : public home::Device, public boost::enable_shared_from_this<Device>
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
			friend class DeviceController;

			Ref<Device> device;
			size_t interval;
			boost::asio::deadline_timer timer;
			home::TimerCallback<home::Device>* callback;

			void TimerCallback(const boost::system::error_code& err);

		public:
			friend class DeviceController;

			Timer(Ref<Device> device, home::TimerCallback<home::Device>* callback);
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

			Ref<Device> device;
			home::EventCallback<home::Device>* callback;

		public:
			Event(Ref<Device> device, home::EventCallback<home::Device>* callback);
			~Event();

			virtual void Invoke() override;
		};

		boost::shared_mutex mutex;

		std::string name;
		const identifier_t deviceID;
		Ref<DeviceController> controller;
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

		virtual Ref<home::Event> AddEvent(const std::string& id, home::EventCallback<home::Device>* callback) override;
		virtual bool RemoveEvent(const std::string& id) override;

		virtual Ref<home::Timer> AddTimer(const std::string& id, home::TimerCallback<home::Device>* callback) override;
		virtual bool RemoveTimer(const std::string& id) override;

		// Cache for properties
		rapidjson::Document snapshot;

		// Plugin
		const Ref<home::DevicePlugin> plugin;

		virtual Ref<home::DevicePlugin> GetPlugin() override { return plugin; }
		virtual Ref<home::DeviceController> GetController() override;

	public:
		Device(const std::string& name, identifier_t deviceID, Ref<home::DevicePlugin> plugin, Ref<DeviceController> controller, Ref<Room> room);
		~Device();
		static Ref<Device> Create(const std::string& name, identifier_t deviceID, Ref<home::DevicePlugin> plugin, Ref<DeviceController> controller, Ref<Room> room);

		/// @brief Get device name
		/// @return Device name
		std::string GetName();

		/// @brief Set device name
		/// @param v New device name
		bool SetName(const std::string& v);

		inline identifier_t GetDeviceID() { return deviceID; }

		/// @brief Get device controller
		/// @return Device controller (can be null)
		Ref<DeviceController> GetController2();

		/// @brief Set device controller
		/// @param v Device controller (can be null)
		/// @return Successfulness
		bool SetController(Ref<DeviceController> v);

		/// @brief Get room
		/// @return Room (can be null)
		Ref<Room> GetRoom();

		/// @brief Set room
		/// @param v Room (can be null)
		/// @return Successfulness
		bool SetRoom(Ref<Room> v);

		/// @brief Get device plugin id
		/// @return Device plugin id
		inline identifier_t GetPluginID() { return plugin->GetPluginID(); }

		/// @brief Invoke plugin event
		/// @param event Event name
		void Invoke(const std::string& event);

		/// @brief Take property snapshot
		void TakeSnapshot();
	};
}