#include "JsonApi.hpp"
#include "../../plugin/PluginManager.hpp"

namespace server
{
	// Plugins
	void JsonApi::BuildJsonPlugins(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(output.IsObject());

		Ref<PluginManager> pluginManager = PluginManager::GetInstance();
		assert(pluginManager != nullptr);

		boost::shared_lock_guard lock(pluginManager->mutex);

		output.AddMember("timestamp", rapidjson::Value(pluginManager->timestamp), allocator);

		output.CopyFrom(pluginManager->snapshot, allocator);
	}
}