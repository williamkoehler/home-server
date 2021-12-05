#pragma once
#include "../common.hpp"
#include "Home.hpp"

namespace server
{
	class Device;
	class DeviceController;
	class Action;

	class Database;

	class JsonApi;

	class Room : public boost::enable_shared_from_this<Room>
	{
	private:
		friend class Home;
		friend class Database;
		friend class JsonApi;

		boost::shared_mutex mutex;

		std::string name;
		const identifier_t roomID;
		std::string type;

		boost::container::set<identifier_t> deviceList;
		boost::container::set<identifier_t> deviceControllerList;
		boost::container::set<identifier_t> actionList;

	public:
		Room(const std::string& name, identifier_t roomID, const std::string& type);
		~Room();
		static Ref<Room> Create(const std::string& name, identifier_t roomID, const std::string& type);

		std::string GetName();
		bool SetName(const std::string& v);

		inline uint32_t GetRoomID() { return roomID; }

		std::string GetType();
		bool SetType(const std::string& v);

		//! Device

		/// @brief Add device to room
		/// @param device Device
		bool AddDevice(Ref<Device> device);

		/// @brief Get device count
		/// @return Device count
		inline size_t GetDeviceCount()
		{
			boost::shared_lock_guard lock(mutex);
			return deviceList.size();
		}

		/// @brief Remove device from room
		/// @param device Device
		bool RemoveDevice(Ref<Device> device);

		//! DeviceController

		/// @brief Add device to room
		/// @param controller Device controller
		bool AddDeviceController(Ref<DeviceController> controller);

		/// @brief Get device controller count
		/// @return Device controller count
		inline size_t GetDeviceControllerCount()
		{
			boost::shared_lock_guard lock(mutex);
			return deviceControllerList.size();
		}

		/// @brief Remove device controller from room
		/// @param controller Device controller
		bool RemoveDeviceController(Ref<DeviceController> controller);

		//! Action

		/// @brief Add action to room
		/// @param action Action
		bool AddAction(Ref<Action> action);

		/// @brief Get action count
		/// @return Action count
		inline size_t GetActionCount()
		{
			boost::shared_lock_guard lock(mutex);
			return actionList.size();
		}

		/// @brief Remove action from room
		/// @param action Action
		bool RemoveAction(Ref<Action> action);
	};
}