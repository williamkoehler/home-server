#include "JsonApi.h"
#include "../Core.h"

#include "../plugin/PluginManager.h"
#include <Script.h>

#include "../user/UserManager.h"
#include "../user/User.h"

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