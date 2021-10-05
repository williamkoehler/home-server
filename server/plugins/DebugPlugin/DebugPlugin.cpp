#include <Plugin.hpp>
#include "DebugLight.hpp"
#include "DebugVisuals.hpp"

extern "C"
{
	void RegisterPlugin(Ref<home::Plugin> plugin)
	{
		plugin->RegisterDeviceScript<DebugVisuals>();
		plugin->RegisterDeviceScript<DebugLight>();
	}
}