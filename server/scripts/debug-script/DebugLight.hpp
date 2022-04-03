#pragma once
#include "common.hpp"
#include <home-scripting/tools/Http.hpp>
#include <home-scripting/tools/Timer.hpp>

class DebugLight : public native::NativeScript
{
  public:
    DebugLight(Ref<scripting::View> view, Ref<native::NativeScriptSource> scriptSource);

    Ref<Property> power;

    virtual bool InitializeScript() override;

    bool Callback(Ref<Controller> controller);

    virtual bool TerminateScript() override;
};