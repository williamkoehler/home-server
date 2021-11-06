#include "JsonApi.hpp"
#include "../../plugin/PluginManager.hpp"

namespace server
{
	// Plugins
	void JsonApi::ProcessJsonGetPluginsMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		BuildJsonPlugins(output, allocator);
	}
}