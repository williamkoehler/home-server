#pragma once
#include "LibraryInformations.hpp"
#include "common.hpp"
#include "utils/NativeEvent.hpp"
#include <home-scripting/Script.hpp>
#include <home-scripting/utils/Property.hpp>

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

                virtual bool Invoke(const std::string& event, Ref<EventCaller> caller) override;

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

                template <class T>
                inline Ref<Event> AddEvent(const std::string& id, bool (T::*callback)(Ref<EventCaller>))
                {
                    return AddEvent(id, (EventCallbackConversion<T>{callback}).function);
                }

                bool RemoveEvent(const std::string& id);
                void ClearEvents();

                virtual bool InitializeScript() = 0;
                virtual bool TerminateScript() = 0;
            };

            using CreateScriptCallback = Ref<NativeScript>(Ref<View> view, Ref<NativeScriptSource> scriptSource);
        }
    }
}