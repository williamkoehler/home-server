#pragma once
#include "../common.hpp"
#include <PluginManager.hpp>
#include <DevicePlugin.hpp>
#include <DeviceControllerPlugin.hpp>
#include <boost/dll.hpp>

namespace server
{
	class JsonApi;

	class PluginManager : public home::PluginManager, public boost::enable_shared_from_this<PluginManager>
	{
	private:
		friend class JsonApi;

		boost::shared_mutex mutex;

		boost::container::vector<Ref<boost::dll::shared_library>> libraryList;

		// Device plugin
		struct DevicePluginReference
		{
			home::DevicePluginDescription description;
			home::CreateDevicePluginFunction* createFunction;
		};
		boost::unordered_map<identifier_t, DevicePluginReference> devicePluginList;

		// Device controller plugin
		struct DeviceControllerPluginReference
		{
			home::DeviceControllerPluginDescription description;
			home::CreateDeviceControllerPluginFunction* createFunction;
		};
		boost::unordered_map<identifier_t, DeviceControllerPluginReference> deviceControllerPluginList;

		// Json snapshot of device plugin, and device controller plugin descriptions
		rapidjson::Document snapshot;

		/// @brief Take a json snaphot
		void TakeSnapshot();

		/// @brief Loads plugin from plugins folder
		/// @param name Plugin name/ filename
		void LoadPlugin(std::string name);

		// IO
		void Load();
		static void SaveDefault();

	public:
		PluginManager();
		~PluginManager();
		static Ref<PluginManager> Create();
		static Ref<PluginManager> GetInstance();

		virtual bool RegisterDevicePlugin(home::DevicePluginDescription description, home::CreateDevicePluginFunction* createFunction) override;
		Ref<home::DevicePlugin> CreateDevicePlugin(identifier_t pluginID);

		virtual bool RegisterDeviceControllerPlugin(home::DeviceControllerPluginDescription description, home::CreateDeviceControllerPluginFunction* createFunction) override;
		Ref<home::DeviceControllerPlugin> CreateDeviceControllerPlugin(identifier_t pluginID);
	};
}