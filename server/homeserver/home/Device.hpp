#pragma once
#include "../common.hpp"
#include "Home.hpp"
#include "Room.hpp"
#include <DevicePlugin.hpp>

namespace server
{
	class Database;

	class JsonApi;

	class Device : public boost::enable_shared_from_this<Device>
	{
	private:
		friend class Home;
		friend class Room;
		friend class Database;
		friend class JsonApi;

		boost::shared_mutex mutex;

		std::string name;
		const identifier_t deviceID;
		Ref<DeviceController> controller;
		Ref<Room> room;

		rapidjson::Document snapshot;

		boost::mutex pluginMutex;
		const Ref<home::DevicePlugin> plugin;

	public:
		Device(const std::string& name, identifier_t deviceID, Ref<home::DevicePlugin> plugin, Ref<DeviceController> controller, Ref<Room> room);
		~Device();

		/// @brief Get device name
		/// @return Device name
		inline std::string GetName()
		{
			boost::lock_guard lock(mutex);
			return name;
		}

		/// @brief Set device name
		/// @param v New device name
		inline void SetName(const std::string& v)
		{
			boost::lock_guard lock(mutex);
			Home::GetInstance()->UpdateTimestamp();
			name = v;
		}

		inline identifier_t GetDeviceID() { return deviceID; }

		/// @brief Get room
		/// @return Room (can be null)
		inline Ref<Room> GetRoom()
		{
			boost::shared_lock_guard lock(mutex);
			return room;
		}

		/// @brief Set room
		/// @param v Room (can be null)
		inline void SetRoom(Ref<Room> v)
		{
			boost::lock_guard lock(mutex);
			if (room != nullptr)
				room->RemoveDevice(shared_from_this());

			room = v;

			if (room != nullptr)
				room->AddDevice(shared_from_this());
		}

		/// @brief Get device plugin
		/// @return Device plugin
		inline Ref<home::DevicePlugin> GetPlugin() { return plugin; }
		/// @brief Get device plugin id
		/// @return Device plugin id
		inline identifier_t GetPluginID() { return plugin->GetPluginID(); }
		/// @brief Get plugin mutex
		/// @return Plugin mutex
		inline boost::mutex& GetPluginMutex() { return pluginMutex; }

		/// @brief Update device plugin
		/// @param signalManager SignalManager
		/// @param cycle Current singal cycle
		void Update(size_t cycle);
	};
}