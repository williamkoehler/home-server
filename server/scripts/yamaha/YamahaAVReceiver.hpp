#pragma once
#include "common.hpp"
#include <home-scripting/tools/Http.hpp>

class YamahaAVReceiver : public native::NativeScript
{
  public:
    YamahaAVReceiver(Ref<scripting::View> view, Ref<native::NativeScriptSource> scriptSource);
    static Ref<YamahaAVReceiver> Create(Ref<scripting::View> view, Ref<native::NativeScriptSource> scriptSource);

    Ref<Property> power;

    virtual bool InitializeScript() override;

    bool OnHttpResponse(Ref<Controller> controller);

    virtual bool TerminateScript() override;
};