#pragma once
#include "LibraryInformations.hpp"
#include "common.hpp"
#include <home-scripting/Script.hpp>
#include <home-scripting/utils/Event.hpp>
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

                Ref<Event> AddEvent(const std::string& id, EventMethod<> event);

                template <class T>
                inline Ref<Event> AddEvent(const std::string& id, EventMethod<> event)
                {
                    return AddEvent(id, EventMethodConversion<T>{event}.f2);
                }

                bool RemoveEvent(const std::string& id);
                void ClearEvents();

                virtual bool Init() = 0;
            };

            using CreateScriptCallback = void(Ref<View> view, Ref<NativeScriptSource> scriptSource, Ref<NativeScript>* result);
        }
    }
}