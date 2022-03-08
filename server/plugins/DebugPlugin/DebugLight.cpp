#include "DebugLight.hpp"

const id_t STATE = 1;
const id_t DIMMABLE = 2;

bool DebugLight::Initialize(Ref<home::Device> device)
{
	device->AddAttribute("visuals",
R"([
  {
    "type": "light-switch",
    "name": "State",
    "property": "power"
  },
  {
	  "type": "text-field",
	  "name": "Text Field",
	  "property": "text"
  }
])");

	device->AddProperty("power", home::BooleanProperty::Create());
	device->AddProperty("color", home::ColorProperty::Create());
	device->AddProperty("address", home::EndpointProperty::Create());
	device->AddProperty("text", home::StringProperty::Create());

	device->AddEvent<DebugLight>("disco", &DebugLight::Disco);

	Ref<home::Timer> timer = device->AddTimer<DebugLight>("disco", &DebugLight::Disco);
	if (timer != nullptr)
		timer->Start(30);

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
