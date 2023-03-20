#pragma once
#include "LibraryInformation.hpp"
#include "common.hpp"
#include "utils/Method.hpp"
#include "utils/Property.hpp"
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

            class NativeScript : public Script
            {
              private:
                UniqueRef<NativeScriptImpl> scriptImpl;
                robin_hood::unordered_node_map<std::string, Method> methodList;
                robin_hood::unordered_node_map<std::string, Property> propertyList;

              public:
                NativeScript(Ref<View> view, Ref<NativeScriptSource> scriptSource,
                             UniqueRef<NativeScriptImpl> scriptImpl);
                virtual ~NativeScript();
                static Ref<Script> Create(Ref<View> view, Ref<NativeScriptSource> scriptSource);

                bool AddAttribute(const std::string& name, const char* json);
                bool RemoveAttribute(const std::string& name);
                void ClearAttributes();

                bool AddProperty(const std::string& name, const Property& property);
                bool RemoveProperty(const std::string& name);
                void ClearProperties();

                bool AddMethod(const std::string& name, const Method& method);
                bool RemoveMethod(const std::string& name);
                void ClearMethods();

                Event AddEvent(const std::string& name);
                bool RemoveEvent(const std::string& name);
                void ClearEvents();

                bool Initialize() override;

                virtual Value GetProperty(const std::string& name) override;
                virtual void SetProperty(const std::string& name, const Value& value) override;

                bool Invoke(const std::string& name, const Value& parameter) override;

                virtual void JsonGetState(rapidjson::Value& output,
                                          rapidjson::Document::AllocatorType& allocator) override;
                virtual void JsonSetState(rapidjson::Value& input) override;
            };

            class NativeScriptImpl
            {
              private:
                friend class server::scripting::native::NativeScript;

                Ref<NativeScript> script;

              public:
                NativeScriptImpl()
                {
                }
                virtual ~NativeScriptImpl()
                {
                }

                constexpr virtual ViewType GetViewType() const = 0;

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
                /// @return Successfulness
                inline bool AddProperty(const std::string& name, const Property& property)
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
                /// @param method Method
                /// @return Successfulness
                inline bool AddMethod(const std::string& name, const Method& method)
                {
                    return script->AddMethod(name, method);
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

                /// @brief Add event
                ///
                /// @param name Event name
                /// @return Ref<Event> Event instance
                inline Event AddEvent(const std::string& name)
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