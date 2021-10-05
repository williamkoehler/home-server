#pragma once
#include "../common.hpp"
#include <home/Home.hpp>
#include <home/Room.hpp>
#include <home/Device.hpp>
#include <home/DeviceManager.hpp>
#include <home/Action.hpp>

namespace server
{
	class Core;
	class DeviceManager;
	class Room;
	class Device;
	class Action;

	class SignalManager;

	class JsonApi;
	class SSHSession;

	class Home : public home::Home, public boost::enable_shared_from_this<Home>
	{
	private:
		friend class Core;
		friend class JsonApi;
		friend class SSHSession;

		boost::shared_mutex mutex;

		boost::atomic<time_t> timestamp = 0;

		boost::unordered::unordered_map<uint32_t, Ref<Room>> roomList;
		boost::unordered::unordered_map<uint32_t, Ref<Device>> deviceList;
		boost::unordered::unordered_map<uint32_t, Ref<DeviceManager>> deviceManagerList;
		boost::unordered::unordered_map<uint32_t, Ref<Action>> actionList;

		size_t cycleCount = SIZE_MAX;

		//Workers
		boost::mutex workerMutex;
		boost::container::vector<WeakRef<DeviceManager>> deviceManagerUpdateList;
		boost::container::vector<WeakRef<Device>> deviceUpdateList;
		void Worker();

		// IO
		void Load();

	public:
		Home();
		~Home();
		static Ref<Home> Create();
		static Ref<Home> GetInstance();

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
		/// @param roomID Room id
		/// @param type Room type
		/// @param json JSON
		/// @return 
		Ref<Room> AddRoom(std::string name, uint32_t roomID, uint16_t type, rapidjson::Value& json);

		/// @brief Get room count
		/// @return Room count
		virtual inline size_t GetRoomCount() override
		{
			boost::shared_lock_guard lock(mutex);
			return roomList.size();
		}

		/// @brief Prefer using Home::GetRoom_
		virtual Ref<home::Room> GetRoom(uint32_t roomID) override;

		/// @brief Get room using room id
		/// @param roomID Room id
		/// @return Room or nullptr
		Ref<Room> GetRoom_(uint32_t roomID);

		/// @brief Remove room using room id
		/// @param roomID Room id
		void RemoveRoom(uint32_t roomID);

		//! Device

		/// @brief Add device
		/// @param name Device name
		/// @param deviceID Device id
		/// @param scriptID Device script id
		/// @param json JSON
		/// @return 
		Ref<Device> AddDevice(std::string name, uint32_t deviceID, uint32_t scriptID, rapidjson::Value& json);

		/// @brief Get device count
		/// @return 
		virtual inline size_t GetDeviceCount() override
		{
			boost::shared_lock_guard lock(mutex);
			return deviceList.size();
		}

		/// @brief Prefer using Home::GetDevice_
		virtual Ref<home::Device> GetDevice(uint32_t deviceID) override;

		/// @brief Get device using device id
		/// @param deviceID Device id
		/// @return Device or nullptr
		Ref<Device> GetDevice_(uint32_t deviceID);

		/// @brief Remove device using device id
		/// @param deviceID Device id
		void RemoveDevice(uint32_t deviceID);

		/// @brief Add device to update list
		/// @param device Device
		void AddDeviceToUpdatables(Ref<Device> device);

		//! DeviceManager

		/// @brief Add device manager
		/// @param name Device manager name
		/// @param managerID Device manager id
		/// @param scriptID Device manager script id
		/// @param json JSON
		/// @return 
		Ref<DeviceManager> AddDeviceManager(std::string name, uint32_t managerID, uint32_t scriptID, rapidjson::Value& json);

		/// @brief Get device manager count
		/// @return 
		virtual inline size_t GetDeviceManagerCount() override
		{
			boost::shared_lock_guard lock(mutex);
			return deviceManagerList.size();
		}

		/// @brief Prefer using Home::GetDeviceManager_
		virtual Ref<home::DeviceManager> GetDeviceManager(uint32_t managerID) override;

		/// @brief Get device manager using device manager id
		/// @param managerID Device manager id
		/// @return Device manager or nullptr
		Ref<DeviceManager> GetDeviceManager_(uint32_t managerID);

		/// @brief Remove device manager using id
		/// @param managerID Device manager id
		void RemoveDeviceManager(uint32_t managerID);

		/// @brief Add device manager to update list
		/// @param deviceManager Device manager
		void AddDeviceManagerToUpdatables(Ref<DeviceManager> deviceManager);

		//! Action

		/// @brief Add action
		/// @param name Action name
		/// @param actionID Action id
		/// @param sourceID Action script source id
		/// @param json JSON
		/// @return 
		Ref<Action> AddAction(std::string name, uint32_t actionID, uint32_t sourceID, rapidjson::Value& json);

		/// @brief Get action count
		/// @return Action count
		virtual inline size_t GetActionCount() override
		{
			boost::shared_lock_guard lock(mutex);
			return actionList.size();
		}

		/// @brief Prefer using Home::GetAction_
		virtual Ref<home::Action> GetAction(uint32_t actionID) override;

		/// @brief Get action using action id
		/// @param actionID Action id
		/// @return Action or nullptr
		Ref<Action> GetAction_(uint32_t actionID);

		/// @brief Remove action using action id
		/// @param actionID Action id
		void RemoveAction(uint32_t actionID);

		/// @brief Update devices, device managers, etc...
		void Update();

		/// @brief Save home to JSON file
		void Save();
	};
}