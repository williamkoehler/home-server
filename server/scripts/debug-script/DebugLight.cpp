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
    //     device.AddAttribute("visuals",
    //                         R"([
    //   {
    //     "type": "light-switch",
    //     "name": "State",
    //     "property": "power"
    //   },
    //   {
    // 	  "type": "text-field",
    // 	  "name": "Text Field",
    // 	  "property": "text"
    //   }
    // ])");

    //     power = home::BooleanProperty::Create();
    //     device.AddProperty("power", power);

    //     device.AddProperty("color", home::ColorProperty::Create());
    //     device.AddProperty("address", home::EndpointProperty::Create());
    //     device.AddProperty("text", home::StringProperty::Create());

    //     device.AddEvent<DebugLight>("disco", &DebugLight::Disco);

    //     Ref<home::Timer> timer = device.AddTimer<DebugLight>("disco", &DebugLight::Disco);
    //     if (timer != nullptr)
    //         timer->Start(30);

    return true;
}

bool DebugLight::Disco()
{
    LOG_INFO("Disco {0}", power->GetBoolean());
    return true;
}

bool DebugLight::TerminateScript()
{
    return true;
}