#pragma once
#include "../common.h"
#include <home/Home.h>
#include <home/Room.h>
#include <home/Device.h>
#include <home/DeviceManager.h>
#include <home/Action.h>

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

		//Timestamp
		void UpdateTimestamp();
		inline time_t GetLastTimestamp()
		{
			return timestamp;
		}

		//Room
		Ref<Room> AddRoom(std::string name, uint32_t roomID, uint16_t type, rapidjson::Value& json);

		virtual inline size_t GetRoomCount() override
		{
			boost::shared_lock_guard lock(mutex);
			return roomList.size();
		}
		virtual Ref<home::Room> GetRoom(uint32_t roomID) override;
		Ref<Room> GetRoom_(uint32_t roomID);

		void RemoveRoom(uint32_t roomID);

		//Device
		Ref<Device> AddDevice(std::string name, uint32_t deviceID, uint32_t scriptID, rapidjson::Value& json);

		virtual inline size_t GetDeviceCount() override
		{
			boost::shared_lock_guard lock(mutex);
			return deviceList.size();
		}
		virtual Ref<home::Device> GetDevice(uint32_t deviceID) override;
		Ref<Device> GetDevice_(uint32_t deviceID);

		void RemoveDevice(uint32_t deviceID);

		void AddDeviceToUpdatables(Ref<Device> device);

		//DeviceManager
		Ref<DeviceManager> AddDeviceManager(std::string name, uint32_t managerID, uint32_t scriptID, rapidjson::Value& json);

		virtual inline size_t GetDeviceManagerCount() override
		{
			boost::shared_lock_guard lock(mutex);
			return deviceManagerList.size();
		}
		virtual Ref<home::DeviceManager> GetDeviceManager(uint32_t managerID) override;
		Ref<DeviceManager> GetDeviceManager_(uint32_t managerID);

		void RemoveDeviceManager(uint32_t managerID);

		void AddDeviceManagerToUpdatables(Ref<DeviceManager> deviceManager);

		//Action
		Ref<Action> AddAction(std::string name, uint32_t actionID, uint32_t sourceID, rapidjson::Value& json);

		virtual inline size_t GetActionCount() override
		{
			boost::shared_lock_guard lock(mutex);
			return actionList.size();
		}
		virtual Ref<home::Action> GetAction(uint32_t actionID) override;
		Ref<Action> GetAction_(uint32_t actionID);

		void RemoveAction(uint32_t actionID);

		//
		void Update();

		//IO
		void Save();
	};
}