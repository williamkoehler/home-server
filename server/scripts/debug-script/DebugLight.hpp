#pragma once
#include "common.hpp"

class DebugLight : public server::scripting::native::NativeScript
{
  public:
    DebugLight(Ref<scripting::View> view, Ref<native::NativeScriptSource> scriptSource);
    // DEVICEPLUGIN_DESCRIPTION(DebugLight, "Debug Light", "Debug light that does not control anything.");
    static Ref<DebugLight> Create(Ref<scripting::View> view, Ref<native::NativeScriptSource> scriptSource);

    Ref<BooleanProperty> power;

    virtual bool InitializeScript() override;

    bool Disco();

    virtual bool TerminateScript() override;
};