#pragma once
#include <home/DevicePlugin.hpp>

class DebugLight : public home::DevicePlugin
{
public:
	DEVICEPLUGIN_DESCRIPTION(DebugLight, "Debug Light", "Debug light that does not control anything.");

	bool state = false;

	virtual bool Initialize(Ref<home::Device> device) override;

	bool Disco(Ref<home::Device> device);

	virtual bool Terminate(Ref<home::Device> device) override;
};

