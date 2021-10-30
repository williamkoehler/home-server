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
		std::string GetName();

		/// @brief Set device name
		/// @param v New device name
		bool SetName(const std::string& v);

		inline identifier_t GetDeviceID() { return deviceID; }

		/// @brief Get room
		/// @return Room (can be null)
		Ref<Room> GetRoom();

		/// @brief Set room
		/// @param v Room (can be null)
		bool SetRoom(Ref<Room> v);

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