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
            class NativeScriptSource;
            class NativeScriptImpl;

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
                bool RemoveMethod(const std::string& name);
                void ClearMethods();

                Ref<Event> AddEvent(const std::string& name);
                bool RemoveEvent(const std::string& name);
                void ClearEvents();

                bool Initialize() override;

                bool Invoke(const std::string& name, Ref<Value> parameter) override;
            };

            class NativeScriptImpl
            {
              private:
                friend class NativeScript;

                Ref<NativeScript> script;

              public:
                NativeScriptImpl()
                {
                }
                virtual ~NativeScriptImpl()
                {
                }

                /// @brief Initialize script
                ///
                /// @return Successfulness
                virtual bool Initialize() = 0;

                /// @brief Add attribute
                ///
                /// @param name Attribute name
                /// @param json Attribute value (in json format)
                /// @return Successfulness
                inline bool AddAttribute(const std::string& name, const char* json)
                {
                    return script->AddAttribute(name, json);
                }

                /// @brief Remove attributes
                ///
                /// @param name Attribute name
                /// @return Successfulness
                inline bool RemoveAttribute(const std::string& name)
                {
                    return script->RemoveAttribute(name);
                }

                /// @brief Clear attributes
                ///
                void ClearAttributes()
                {
                    return script->ClearAttributes();
                }

                /// @brief Add propertry
                ///
                /// @param name Property name
                /// @param property Value
                /// @return Ref<Value> Value
                inline Ref<Value> AddProperty(const std::string& name, Ref<Value> property)
                {
                    return script->AddProperty(name, property);
                }

                /// @brief Remove property
                ///
                /// @param name Property name
                /// @return Successfulness
                inline bool RemoveProperty(const std::string& name)
                {
                    return script->RemoveProperty(name);
                }

                /// @brief Clear properties
                ///
                inline void ClearProperties()
                {
                    script->ClearProperties();
                }

                /// @brief Add method
                ///
                /// @param name Method name
                /// @param callback Method callback
                /// @return Successfulness
                inline bool AddMethod(const std::string& name, MethodCallback<> callback)
                {
                    return script->AddMethod(name, callback);
                }

                /// @brief Remove method
                ///
                /// @param name Method name
                /// @return Successfulness
                inline bool RemoveMethod(const std::string& name)
                {
                    return script->RemoveMethod(name);
                }

                /// @brief Clear methods
                ///
                inline void ClearMethods()
                {
                    return script->ClearMethods();
                }

                template <class T>
                inline bool AddMethod(const std::string& name, MethodCallback<T> callback)
                {
                    return script->AddMethod(name, MethodCallbackConversion<T>{callback}.f2);
                }

                /// @brief Add event
                ///
                /// @param name Event name
                /// @return Ref<Event> Event instance
                inline Ref<Event> AddEvent(const std::string& name)
                {
                    return script->AddEvent(name);
                }

                /// @brief Remove event
                ///
                /// @param name Event name
                /// @return Successfulness
                inline bool RemoveEvent(const std::string& name)
                {
                    return script->RemoveEvent(name);
                }

                /// @brief Clear events
                ///
                inline void ClearEvents()
                {
                    script->ClearEvents();
                }

                /// @brief Add timer
                ///
                /// @param method Method to call
                /// @param interval Interval in seconds
                inline void AddTimerTask(const std::string& method, size_t interval)
                {
                    script->AddTimerTask(method, interval);
                }
            };
        }
    }
}