#pragma once
#include "LibraryInformation.hpp"
#include "common.hpp"
#include <home-scripting/Script.hpp>
#include <home-scripting/utils/Method.hpp>
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

                bool AddAttribute(const std::string& name, const char* json);
                bool RemoveAttribute(const std::string& name);
                void ClearAttributes();

                Ref<Property> AddProperty(const std::string& name, Ref<Property> property);
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

                virtual bool Init() = 0;
            };
        }
    }
}