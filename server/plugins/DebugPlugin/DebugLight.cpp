#include "DebugLight.hpp"

const id_t STATE = 1;
const id_t DIMMABLE = 2;

bool DebugLight::Initialize(Ref<home::Device> device)
{
	device->AddAttribute("visuals",
		R"([)"
		R"(    {)"
		R"(        "name": "State",)"
		R"(        "type": "light-switch",)"
		R"(        "value": {)"
		R"(            "type": "bool",)"
		R"(		    "id": 1)"
		R"(        })"
		R"(    })"
		R"(])");

	device->AddProperty("power", home::BooleanProperty::Create());
	device->AddProperty("color", home::ColorProperty::Create());
	device->AddProperty("address", home::EndpointProperty::Create());

	device->AddEvent<DebugLight>("disco", &DebugLight::Disco);

	Ref<home::Timer> timer = device->AddTimer<DebugLight>("disco", &DebugLight::Disco);
	if (timer != nullptr)
		timer->Start(10);

	return true;
}

bool DebugLight::Disco(Ref<home::Device> device)
{
	LOG_INFO("Disco");
	return true;
}

bool DebugLight::Terminate(Ref<home::Device> device)
{
	return true;
}
