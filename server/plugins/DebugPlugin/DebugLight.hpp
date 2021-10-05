#pragma once
#include <home/Home.hpp>
#include <home/DeviceScript.hpp>

class DebugLight : public home::DeviceScript
{
	struct Fields
	{
		bool state = false;
	};

	DEVICESCRIPT_CLASS(
		DebugLight, 
		"Debug Light",
		home::DeviceType::kLightDeviceType, 
		Fields);

	static void OnRegister(home::DeviceScriptDescription* description);

	virtual bool OnInitialize() override;
	virtual bool OnTerminate() override;

	virtual bool HasError() override;
	virtual std::string GetError() override;

	virtual void OnUpdate(Ref<home::SignalManager> signalManager, size_t cycle) override;

	virtual void GetFields(home::WriteableFieldCollection& collection) override;
	virtual bool GetField(home::WriteableField& field) override;
	virtual bool SetField(home::ReadableField& field) override;
};

