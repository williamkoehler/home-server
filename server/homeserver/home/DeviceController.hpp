#pragma once
#include "../common.hpp"
#include "Home.hpp"
#include "Room.hpp"
#include <DeviceControllerPlugin.hpp>

namespace server
{
	class Database;

	class JsonApi;

	class DeviceController : public boost::enable_shared_from_this<DeviceController>
	{
	private:
		friend class Home;
		friend class Room;
		friend class Database;
		friend class JsonApi;

		boost::shared_mutex mutex;

		std::string name;
		const identifier_t controllerID;
		Ref<Room> room;

		rapidjson::Document snapshot;

		boost::mutex pluginMutex;
		const Ref<home::DeviceControllerPlugin> plugin;

	public:
		DeviceController(const std::string& name, identifier_t controllerID, Ref<home::DeviceControllerPlugin> plugin, Ref<Room> room);
		~DeviceController();

		/// @brief Get device controller name
		/// @return Device name
		inline std::string GetName()
		{
			boost::lock_guard lock(mutex);
			return name;
		}

		/// @brief Set device controller name
		/// @param v New device name
		inline void SetName(std::string v)
		{
			boost::lock_guard lock(mutex);
			Home::GetInstance()->UpdateTimestamp();
			name = std::move(v);
		}

		inline identifier_t GetDeviceControllerID() { return controllerID; }

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
				room->RemoveDeviceController(shared_from_this());

			room = v;

			if (room != nullptr)
				room->AddDeviceController(shared_from_this());
		}

		/// @brief Get device controller plugin
		/// @return Device controller plugin
		inline Ref<home::DeviceControllerPlugin> GetPlugin() { return plugin; }
		/// @brief Get device controller plugin id
		/// @return Device controller plugin id
		inline identifier_t GetPluginID() { return plugin->GetPluginID(); }
		/// @brief Get plugin mutex
		/// @return Plugin mutex
		inline boost::mutex& GetPluginMutex() { return pluginMutex; }

		/// @brief Update device plugin
		/// @param cycle Current cycle
		void Update(size_t cycle);
	};
}