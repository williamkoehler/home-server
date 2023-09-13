#pragma once
#include "library_information.hpp"
#include "native_script_implementation.hpp"
#include "common.hpp"
#include "interface/method.hpp"
#include "interface/property.hpp"
#include <scripting/script.hpp>
#include <scripting/interface/event.hpp>
#include <scripting/value.hpp>

namespace server
{
    namespace scripting
    {
        namespace native
        {
            class NativeScriptSource;

            class NativeScript : public Script, private Context
            {
              private:
                /// @brief Script C++ implementation
                ///
                Ref<NativeScriptImplementation> scriptImplementation;

                /// @brief Script methods
                ///
                robin_hood::unordered_node_map<std::string, UniqueRef<Method>> methodMap;

                /// @brief Script properties
                ///
                robin_hood::unordered_node_map<std::string, UniqueRef<Property>> propertyMap;

                virtual bool AddAttribute(const std::string& name, const char* json) override;
                virtual bool RemoveAttribute(const std::string& name) override;
                virtual void ClearAttributes() override;

                virtual bool AddProperty(const std::string& name, UniqueRef<Property> property) override;
                virtual bool RemoveProperty(const std::string& name) override;
                virtual void ClearProperties() override;

                virtual bool AddMethod(const std::string& name, UniqueRef<Method> method) override;
                virtual bool RemoveMethod(const std::string& name) override;
                virtual void ClearMethods() override;

                virtual Event AddEvent(const std::string& name) override;
                virtual bool RemoveEvent(const std::string& name) override;
                virtual void ClearEvents() override;

              public:
                NativeScript(const Ref<View>& view, const Ref<NativeScriptSource>& scriptSource,
                             const Ref<NativeScriptImplementation>& scriptImpl);
                virtual ~NativeScript();
                static Ref<Script> Create(const Ref<View>& view, const Ref<NativeScriptSource>& scriptSource);

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
                                               PropertyFlags propertyFlags = kPropertyFlag_Visible) override;
                virtual PropertyFlags JsonSetProperties(const rapidjson::Value& input,
                                                        PropertyFlags propertyFlags = kPropertyFlag_All) override;
            };
        }
    }
}