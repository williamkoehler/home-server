#include "JsonApi.hpp"
#include "../Core.hpp"

#include "../plugin/PluginManager.hpp"
#include <Script.hpp>

#include "../user/UserManager.hpp"
#include "../user/User.hpp"

namespace server
{
	// Plugins
	void JsonApi::ProcessJsonGetPluginsMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context)
	{
		assert(input.IsObject() && output.IsObject());

		// Build response
		rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

		BuildJsonPlugins(output, allocator);

		BuildJsonAckMessageWS(output);
	}
}