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

    Ref<Timer> timer = Timer::Create(shared_from_this(), &DebugLight::Callback);
    timer->Start(5);

    return true;
}

bool DebugLight::Callback(Ref<Controller> controller)
{
    switch (controller->GetType())
    {
    case ControllerType::kHttpController:
    {
        Ref<HttpController> http = boost::dynamic_pointer_cast<HttpController>(controller);

        LOG_INFO("{0}", http->GetContent());

        break;
    }
    case ControllerType::kTimerController:
    {
        Ref<Timer> timer = boost::dynamic_pointer_cast<Timer>(controller);

        Https::Get(shared_from_this(), "www.google.com", 443, "/", std::string_view(""), &DebugLight::Callback);

        LOG_INFO("Event called.");
    }
    default:
        break;
    }
    return true;
}

bool DebugLight::TerminateScript()
{
    return true;
}