#pragma once
#include "LibraryInformation.hpp"
#include "common.hpp"
#include <home-scripting/Script.hpp>
#include <home-scripting/utils/Event.hpp>
#include <home-scripting/utils/Method.hpp>
#include <home-scripting/utils/Value.hpp>

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

                bool AddAttribute(const std::string& name, const char* json);
                bool RemoveAttribute(const std::string& name);
                void ClearAttributes();

                Ref<Value> AddProperty(const std::string& name, Ref<Value> property);
                bool RemoveProperty(const std::string& name);
                void ClearProperties();

                Ref<Method> AddMethod(const std::string& name, MethodCallback<> callback);

                template <class T>
                inline Ref<Method> AddMethod(const std::string& name, MethodCallback<> callback)
                {
                    return AddMethod(name, MethodCallbackConversion<T>{callback}.f2);
                }

                bool RemoveMethod(const std::string& name);
                void ClearMethods();

                Ref<Event> AddEvent(const std::string& name);

                bool RemoveEvent(const std::string& name);
                void ClearEvents();

                virtual bool Init() = 0;
            };
        }
    }
}