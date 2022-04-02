#pragma once
#include "common.hpp"
#include <home-scripting/tools/Http.hpp>
#include <home-scripting/tools/Timer.hpp>

class DebugLight : public native::NativeScript
{
  public:
    DebugLight(Ref<scripting::View> view, Ref<native::NativeScriptSource> scriptSource);
    // DEVICEPLUGIN_DESCRIPTION(DebugLight, "Debug Light", "Debug light that does not control anything.");
    static Ref<DebugLight> Create(Ref<scripting::View> view, Ref<native::NativeScriptSource> scriptSource);

    Ref<Property> power;

    virtual bool InitializeScript() override;

    bool Disco(Ref<EventCaller> caller);

    virtual bool TerminateScript() override;
};