#pragma once
#include "LibraryInformation.hpp"
#include "common.hpp"
#include <home-scripting/Script.hpp>
#include <home-scripting/utils/Event.hpp>
#include <home-scripting/utils/Value.hpp>

namespace server
{
    namespace scripting
    {
        namespace native
        {
            class NativeScript;
            class NativeScriptSource;

            class NativeScriptImpl
            {
              public:
                Ref<NativeScript> script;

                NativeScriptImpl()
                {
                }
                virtual ~NativeScriptImpl()
                {
                }

                virtual bool Initialize() = 0;
            };

            template <class T = NativeScriptImpl>
            using MethodCallback = bool (T::*)(const std::string& name, Ref<Value> parameter);

            template <class T>
            union MethodCallbackConversion
            {
                MethodCallback<T> f1;
                MethodCallback<> f2;
            };

            class NativeScript : public Script
            {
              private:
                UniqueRef<NativeScriptImpl> scriptImpl;
                robin_hood::unordered_node_map<std::string, MethodCallback<>> methodList;

                bool Initialize() override;

                bool Invoke(const std::string& name, Ref<Value> parameter) override;

              public:
                NativeScript(Ref<View> view, Ref<NativeScriptSource> scriptSource,
                             UniqueRef<NativeScriptImpl> scriptImpl);
                virtual ~NativeScript();
                static Ref<Script> Create(Ref<View> view, Ref<NativeScriptSource> scriptSource);

                bool AddAttribute(const std::string& name, const char* json);
                bool RemoveAttribute(const std::string& name);
                void ClearAttributes();

                Ref<Value> AddProperty(const std::string& name, Ref<Value> property);
                bool RemoveProperty(const std::string& name);
                void ClearProperties();

                bool AddMethod(const std::string& name, MethodCallback<> callback);

                template <class T>
                inline bool AddMethod(const std::string& name, MethodCallback<T> callback)
                {
                    return AddMethod(name, MethodCallbackConversion<T>{callback}.f2);
                }

                bool RemoveMethod(const std::string& name);
                void ClearMethods();

                Ref<Event> AddEvent(const std::string& name);

                bool RemoveEvent(const std::string& name);
                void ClearEvents();
            };
        }
    }
}