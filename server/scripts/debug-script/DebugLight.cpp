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

    Https::Send(shared_from_this(), "disco", "www.google.com", 443, HttpMethod::kGet, "/");

    Ref<Timer> timer = Timer::Create(shared_from_this(), "disco");
    timer->Start(5);

    return true;
}

bool DebugLight::Disco(Ref<EventCaller> caller)
{
    switch (caller->GetType())
    {
    case EventCallerType::kHttpResponseEventCaller: {
        Ref<HttpConnection> http = boost::dynamic_pointer_cast<HttpConnection>(caller);

        LOG_INFO("{0}", http->GetContent());

        break;
    }
    case EventCallerType::kTimerEventCaller: {
        Ref<Timer> timer = boost::dynamic_pointer_cast<Timer>(caller);

        timer->Stop();

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