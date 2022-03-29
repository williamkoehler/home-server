#include "DebugLight.hpp"

const id_t STATE = 1;
const id_t DIMMABLE = 2;

DebugLight::DebugLight(Ref<scripting::View> view, Ref<native::NativeScriptSource> scriptSource)
    : NativeScript(view, scriptSource)
{
}
extern "C" Ref<DebugLight> CreateDebugLight(Ref<scripting::View> view, Ref<native::NativeScriptSource> scriptSource)
{
    return boost::make_shared<DebugLight>(view, scriptSource);
}

bool DebugLight::InitializeScript()
{
    AddAttribute("visuals",
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

    power = AddProperty("power", BooleanProperty::Create());

    AddProperty("color", ColorProperty::Create());
    AddProperty("address", EndpointProperty::Create());
    AddProperty("text", StringProperty::Create());

    AddEvent<DebugLight>("disco", &DebugLight::Disco);

    Ref<Timer> timer = AddTimer("disco", "disco");
    if (timer != nullptr)
        timer->Start(10);

    return true;
}

bool DebugLight::Disco()
{
    power->SetBoolean(!power->GetBoolean());
    LOG_INFO("Disco {0}", power->GetBoolean());
    return true;
}

bool DebugLight::TerminateScript()
{
    return true;
}