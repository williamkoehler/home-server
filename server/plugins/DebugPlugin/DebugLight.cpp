#include "DebugLight.hpp"

const id_t STATE = 1;
const id_t DIMMABLE = 2;

bool DebugLight::Initialize()
{
	cache.SetAttribute("visuals",
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
}

bool DebugLight::Update(Ref<DevicePlugin> controller, size_t cycle)
{
	return false;
}

bool DebugLight::Terminate()
{
	return false;
}
