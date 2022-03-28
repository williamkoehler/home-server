#pragma once
#include "LibraryInformations.hpp"
#include "common.hpp"
#include "utils/NativeEvent.hpp"
#include "utils/NativeTimer.hpp"
#include <home-scripting/Script.hpp>

namespace server
{
    namespace scripting
    {
        namespace native
        {
            class NativeScriptSource;

            class NativeScript : public Script
            {
              private:
                virtual bool Initialize() override;
                virtual bool Terminate() override;

              protected:
                NativeScript(Ref<View> view, Ref<NativeScriptSource> scriptSource);
                virtual ~NativeScript();

                bool AddAttribute(const std::string& id, const char* json);
                bool RemoveAttribute(const std::string& id);
                void ClearAttributes();

                Ref<Property> AddProperty(const std::string& id, Ref<Property> property);
                bool RemoveProperty(const std::string& id);
                void ClearProperties();

                Ref<Event> AddEvent(const std::string& id, EventCallback callback);
                bool RemoveEvent(const std::string& id);
                void ClearEvents();

                Ref<Timer> AddTimer(const std::string& id, TimerCallback callback);
                bool RemoveTimer(const std::string& id);
                void ClearTimers();

                virtual bool InitializeScript() = 0;
                virtual bool TerminateScript() = 0;
            };

            using CreateScriptCallback = Ref<NativeScript>(Ref<View> view, Ref<NativeScriptSource> scriptSource);
        }
    }
}