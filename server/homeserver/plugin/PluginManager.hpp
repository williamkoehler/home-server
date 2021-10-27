#pragma once
#include "../common.hpp"
#include <PluginManager.hpp>
#include <boost/dll.hpp>

namespace server
{
	class JsonApi;

	class PluginManager : public home::PluginManager, public boost::enable_shared_from_this<PluginManager>
	{
	private:
		friend class JsonApi;

		boost::shared_mutex mutex;

		boost::atomic<time_t> timestamp = 0;

		boost::container::vector<Ref<boost::dll::shared_library>> libraryList;

		// Device plugin
		struct DevicePluginReference
		{
			std::string name;
			identifier_t pluginID;
			home::CreateDevicePluginFunction* createFunction;
		};
		boost::unordered_map<uint32_t, DevicePluginReference> devicePluginList;

		// Device controller plugin
		struct DeviceControllerPluginReference
		{
			std::string name;
			identifier_t pluginID;
			home::CreateDeviceControllerPluginFunction* createFunction;
		};
		boost::unordered_map<uint32_t, DeviceControllerPluginReference> deviceControllerPluginList;

		void LoadPlugin(std::string name);

		// IO
		void Load();
		static void SaveDefault();

	public:
		PluginManager();
		~PluginManager();
		static Ref<PluginManager> Create();
		static Ref<PluginManager> GetInstance();

		//! Timestamp

		/// @brief Update timestamp
		void UpdateTimestamp();

		/// @brief Get timestamp
		/// @return Timestamp
		inline time_t GetLastTimestamp()
		{
			return timestamp;
		}

		virtual bool RegisterDevicePlugin(const std::string& name, identifier_t pluginID, home::CreateDevicePluginFunction* createFunction) override;
		Ref<home::DevicePlugin> CreateDevicePlugin(identifier_t pluginID);

		virtual bool RegisterDeviceControllerPlugin(const std::string& name, identifier_t pluginID, home::CreateDeviceControllerPluginFunction* createFunction) override;
		Ref<home::DeviceControllerPlugin> CreateDeviceControllerPlugin(identifier_t pluginID);
	};
}