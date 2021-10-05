#pragma once
#include <home/Home.hpp>
#include <home/DeviceScript.hpp>

class DebugVisuals : public home::DeviceScript
{
	struct Fields
	{
		bool booleanV = false;
		int64_t integerV = 0;
		double_t doubleV = 0.0f;
		std::string stringV = "";
		home::Endpoint endpointV;
		home::Color colorV;
		uint32_t deviceV = 0;
		uint32_t deviceManagerV = 0;
	};

	DEVICESCRIPT_CLASS(
		DebugVisuals,
		"Debug Visuals",
		home::DeviceType::kLightDeviceType,
		Fields);

public:
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