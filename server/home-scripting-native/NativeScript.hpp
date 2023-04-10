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
                /// @brief Script C++ implementation
                ///
                Ref<NativeScriptImpl> scriptImpl;

                /// @brief Script methods
                ///
                robin_hood::unordered_node_map<std::string, UniqueRef<Method>> methodMap;

                /// @brief Script properties
                ///
                robin_hood::unordered_node_map<std::string, UniqueRef<Property>> propertyMap;

              public:
                NativeScript(const Ref<View>& view, const Ref<NativeScriptSource>& scriptSource,
                             const Ref<NativeScriptImpl>& scriptImpl);
                virtual ~NativeScript();
                static Ref<Script> Create(const Ref<View>& view, const Ref<NativeScriptSource>& scriptSource);

                /// @brief Add attribute from json
                ///
                /// @param name Attribute name
                /// @param json Attribute value (in json format)
                /// @return Successfulness
                bool AddAttribute(const std::string& name, const char* json);

                /// @brief Remove attribute
                ///
                /// @param name Attribute name
                /// @return true Attribute was successfuly removed
                /// @return false Attribute does not exist
                bool RemoveAttribute(const std::string& name);

                /// @brief Clear attributes
                ///
                void ClearAttributes();

                /// @brief Add property
                ///
                /// @param name Property name
                /// @param property Property definition
                /// @return Successfulness
                bool AddProperty(const std::string& name, UniqueRef<Property> property);

                /// @brief Remove property
                ///
                /// @param name Property name
                /// @return true Property was successfuly removed
                /// @return false Attribute does not exist
                bool RemoveProperty(const std::string& name);

                /// @brief Clear properties
                ///
                void ClearProperties();

                /// @brief Add method
                ///
                /// @param name Method name
                /// @return Successfulness
                bool AddMethod(const std::string& name, UniqueRef<Method> method);

                /// @brief Remove method
                ///
                /// @param name Method name
                /// @return true Method was sucessfulny removed
                /// @return false Method does not exist
                bool RemoveMethod(const std::string& name);

                /// @brief Remove methods
                ///
                void ClearMethods();

                /// @brief Add event
                ///
                /// @param name Event name
                /// @return Event Event reference
                Event AddEvent(const std::string& name);

                /// @brief Remove event
                ///
                /// @param name Event name
                /// @return true Event was successfuly removed
                /// @return false Event does not exist
                bool RemoveEvent(const std::string& name);

                /// @brief Clear events
                ///
                void ClearEvents();

                /// @brief Initialize script
                ///
                /// @return Successfulness
                bool Initialize() override;

                /// @brief Get property value
                ///
                /// @param name Property name
                /// @return Value Property value
                virtual Value GetProperty(const std::string& name) override;

                /// @brief Set property value
                ///
                /// @param name Property name
                /// @param value Property value
                virtual void SetProperty(const std::string& name, const Value& value) override;

                /// @brief Invoke method
                ///
                /// @param name Method name
                /// @param parameter Paramater
                /// @return true
                /// @return false
                bool Invoke(const std::string& name, const Value& parameter) override;

                virtual void JsonGetProperties(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator,
                                               uint8_t propertyFlags = PropertyFlags::kPropertyFlag_Visible) override;
                virtual uint8_t JsonSetProperties(const rapidjson::Value& input,
                                               uint8_t propertyFlags = PropertyFlags::kPropertyFlags_All) override;
            };

            class NativeScriptImpl : public boost::enable_shared_from_this<NativeScriptImpl>
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

                /// @brief Add attribute from json
                ///
                /// @param name Attribute name
                /// @param json Attribute value (in json format)
                /// @return Successfulness
                inline bool AddAttribute(const std::string& name, const char* json)
                {
                    return script->AddAttribute(name, json);
                }

                /// @brief Remove attribute
                ///
                /// @param name Attribute name
                /// @return true Attribute was successfuly removed
                /// @return false Attribute does not exist
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

                /// @brief Add property
                ///
                /// @param name Property name
                /// @return Successfulness
                inline bool AddProperty(const std::string& name, UniqueRef<Property> property)
                {
                    return script->AddProperty(name, std::move(property));
                }

                /// @brief Remove property
                ///
                /// @param name Property name
                /// @return true Property was successfuly removed
                /// @return false Attribute does not exist
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
                /// @param method Method definition
                /// @return Successfulness
                inline bool AddMethod(const std::string& name, UniqueRef<Method> method)
                {
                    return script->AddMethod(name, std::move(method));
                }

                /// @brief Remove method
                ///
                /// @param name Method name
                /// @return true Method was sucessfulny removed
                /// @return false Method does not exist
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
                /// @return Event Event reference
                inline Event AddEvent(const std::string& name)
                {
                    return script->AddEvent(name);
                }

                /// @brief Remove event
                ///
                /// @param name Event name
                /// @return true Event was successfuly removed
                /// @return false Event does not exist
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

                /// @brief Add timer task
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