#pragma once
#include "../common.hpp"

namespace server
{
	class Device;
	class DeviceController;
	class Room;

	class JsonApi;

	class Home : public boost::enable_shared_from_this<Home>
	{
	private:
		friend class JsonApi;

		boost::shared_mutex mutex;

		boost::atomic<time_t> timestamp = 0;

		boost::unordered::unordered_map<identifier_t, Ref<Room>> roomList;
		boost::unordered::unordered_map<identifier_t, Ref<Device>> deviceList;
		boost::unordered::unordered_map<identifier_t, Ref<DeviceController>> deviceControllerList;

		Ref<boost::asio::io_service> service = nullptr;
		boost::thread worker;

		void Worker();

		// Database
		bool LoadRoom(identifier_t roomID, const std::string& name, const std::string& type);
		bool LoadDeviceController(identifier_t controllerID, const std::string& name, identifier_t pluginID, identifier_t roomID, const std::string& data);
		bool LoadDevice(identifier_t deviceID, const std::string& name, identifier_t pluginID, identifier_t controllerID, identifier_t roomID, const std::string& data);

	public:
		Home();
		~Home();
		static Ref<Home> Create();
		static Ref<Home> GetInstance();

		inline Ref<boost::asio::io_service> GetService() const { return service; }

		//! Timestamp

		/// @brief Update timestamp
		void UpdateTimestamp();

		/// @brief Get timestamp
		/// @return Timestamp
		inline time_t GetLastTimestamp()
		{
			return timestamp;
		}

		//! Room

		/// @brief Add room
		/// @param name Room name
		/// @param type Room type
		/// @param json JSON
		/// @return 
		Ref<Room> AddRoom(const std::string& name, const std::string& type, rapidjson::Value& json);

		/// @brief Get room count
		/// @return Room count
		inline size_t GetRoomCount()
		{
			boost::shared_lock_guard lock(mutex);
			return roomList.size();
		}

		/// @brief Get room using room id
		/// @param roomID Room id
		/// @return Room or nullptr
		Ref<Room> GetRoom(identifier_t roomID);

		/// @brief Remove room using room id
		/// @param roomID Room id
		bool RemoveRoom(identifier_t roomID);

		//! Device

		/// @brief Add device
		/// @param name Device name
		/// @param pluginID Device script id
		/// @param json JSON
		/// @return 
		Ref<Device> AddDevice(const std::string& name, identifier_t pluginID, identifier_t controllerID, identifier_t roomID, rapidjson::Value& json);

		/// @brief Get device count
		/// @return Device count
		inline size_t GetDeviceCount()
		{
			boost::shared_lock_guard lock(mutex);
			return deviceList.size();
		}

		/// @brief Get device using device id
		/// @param deviceID Device id
		/// @return Device or nullptr
		Ref<Device> GetDevice(identifier_t deviceID);

		/// @brief Remove device using device id
		/// @param deviceID Device id
		bool RemoveDevice(identifier_t deviceID);

		//! DeviceController

		/// @brief Add device controller
		/// @param name Device controller name
		/// @param pluginID Device controller script id
		/// @param json JSON
		/// @return 
		Ref<DeviceController> AddDeviceController(const std::string& name, identifier_t pluginID, identifier_t roomID, rapidjson::Value& json);

		/// @brief Get device controller count
		/// @return Device controller count
		inline size_t GetDeviceControllerCount()
		{
			boost::shared_lock_guard lock(mutex);
			return deviceList.size();
		}

		/// @brief Get device controller using controller id
		/// @param controllerID Device controller id
		/// @return Device controller or nullptr
		Ref<DeviceController> GetDeviceController(identifier_t controllerID);

		/// @brief Remove device controller using controller id
		/// @param controllerID Device controller id
		bool RemoveDeviceController(identifier_t controllerID);

		/// @brief Start home worker
		void Run();
	};
}