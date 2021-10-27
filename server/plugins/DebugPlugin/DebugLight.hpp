#pragma once
#include <DevicePlugin.hpp>

class DebugLight : public home::DevicePlugin
{
public:
	DEVICEPLUGIN_DESCRIPTION(DebugLight, "Debug Light");

	bool state = false;

	virtual bool Initialize() override;

	virtual bool Update(Ref<DevicePlugin> controller, size_t cycle) override;

	virtual bool Terminate() override;
};

