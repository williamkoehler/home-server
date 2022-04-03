#include "YamahaAVReceiver.hpp"

extern "C" Ref<YamahaAVReceiver> CreateYamahaAVReceiver(Ref<scripting::View> view,
                                                        Ref<native::NativeScriptSource> scriptSource)
{
    return boost::make_shared<YamahaAVReceiver>(view, scriptSource);
}

YamahaAVReceiver::YamahaAVReceiver(Ref<scripting::View> view, Ref<native::NativeScriptSource> scriptSource)
    : native::NativeScript(view, scriptSource)
{
}

bool YamahaAVReceiver::InitializeScript()
{
    return true;
}

bool YamahaAVReceiver::OnHttpResponse(Ref<Controller> controller)
{
    if (controller->GetType() == ControllerType::kHttpController)
    {
    }
}

bool YamahaAVReceiver::TerminateScript()
{
    return true;
}