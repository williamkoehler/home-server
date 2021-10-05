#pragma once
#include "../common.hpp"
#include <Plugin.hpp>
#include <boost/dll.hpp>

namespace server
{
	class JsonApi;

	class PluginManager : public boost::enable_shared_from_this<PluginManager>, public home::Plugin
	{
	private:
		friend class JsonApi;

		boost::shared_mutex mutex;

		boost::atomic<time_t> timestamp = 0;

		boost::unordered::unordered_map<boost::filesystem::path, Ref<boost::dll::shared_library>> libraryList;

		boost::unordered_map<uint32_t, home::DeviceScriptDescription> deviceScriptList;
		boost::unordered_map<uint32_t, home::DeviceManagerScriptDescription> deviceManagerScriptList;

		void LoadPlugin(std::string name);

		virtual void RegisterDeviceScript(uint32_t scriptID, home::DeviceScriptDescription& scriptDescription) override;
		virtual void RegisterDeviceManagerScript(uint32_t scriptID, home::DeviceManagerScriptDescription& scriptDescription) override;

		// Timestamp
		void UpdateTimestamp();

		// IO
		void Load();
		static void SaveDefault();

	public:
		PluginManager();
		~PluginManager();
		static Ref<PluginManager> Create();
		static Ref<PluginManager> GetInstance();

		// Timestamp
		inline time_t GetLastTimestamp()
		{
			return timestamp;
		}

		Ref<home::DeviceScript> CreateDeviceScript(uint32_t scriptID);
		Ref<home::DeviceManagerScript> CreateDeviceManagerScript(uint32_t scriptID);
	};
}