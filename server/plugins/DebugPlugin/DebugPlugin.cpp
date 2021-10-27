#include <PluginManager.hpp>
#include "DebugLight.hpp"
//#include "DebugVisuals.hpp"

extern "C"
{
	void RegisterPlugins(Ref<home::PluginManager> plugin)
	{
		//plugin->RegisterDeviceScript<DebugVisuals>();
		plugin->RegisterDevicePlugin<DebugLight>();
	}
}