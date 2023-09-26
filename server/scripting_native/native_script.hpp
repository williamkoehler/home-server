#pragma once
#include "common.hpp"
#include <scripting/script.hpp>
#include <scripting_sdk/interface/event.hpp>
#include <scripting_sdk/interface/method.hpp>
#include <scripting_sdk/interface/property.hpp>
#include <scripting_sdk/library_information.hpp>
#include <scripting_sdk/script.hpp>

namespace server
{
    namespace scripting
    {
        namespace native
        {
            class NativeScriptSource;

            class NativeScript : public Script, private sdk::Context
            {
              private:
                /// @brief Script C++ implementation
                ///
                Ref<sdk::Script> scriptImplementation;

                /// @brief Script methods
                ///
                robin_hood::unordered_node_map<std::string, UniqueRef<sdk::Method>> methodMap;

                /// @brief Script properties
                ///
                robin_hood::unordered_node_map<std::string, UniqueRef<sdk::Property>> propertyMap;

                virtual bool AddAttribute(const std::string& name, const char* json) override;
                virtual bool RemoveAttribute(const std::string& name) override;
                virtual void ClearAttributes() override;

                virtual bool AddProperty(const std::string& name, UniqueRef<sdk::Property> property) override;
                virtual bool RemoveProperty(const std::string& name) override;
                virtual void ClearProperties() override;

                virtual bool AddMethod(const std::string& name, UniqueRef<sdk::Method> method) override;
                virtual bool RemoveMethod(const std::string& name) override;
                virtual void ClearMethods() override;

                virtual sdk::Event AddEvent(const std::string& name) override;
                virtual bool RemoveEvent(const std::string& name) override;
                virtual void ClearEvents() override;

              public:
                NativeScript(const Ref<sdk::View>& view, const Ref<NativeScriptSource>& scriptSource,
                             const Ref<sdk::Script>& scriptImpl);
                virtual ~NativeScript();
                static Ref<Script> Create(const Ref<sdk::View>& view, const Ref<NativeScriptSource>& scriptSource);

                /// @brief Initialize script
                ///
                /// @return Successfulness
                bool Initialize() override;

                /// @brief Get property value
                ///
                /// @param name Property name
                /// @return Value Property value
                virtual sdk::MessageBuffer GetProperty(const std::string& name) override;

                /// @brief Set property value
                ///
                /// @param name Property name
                /// @param value Property value
                virtual void SetProperty(const std::string& name, const sdk::Value& value) override;

                /// @brief Invoke method
                ///
                /// @param name Method name
                /// @param parameter Paramater
                /// @return true
                /// @return false
                bool Invoke(const std::string& name, const sdk::Value& parameter) override;

                virtual void JsonGetProperties(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator,
                                               PropertyFlags propertyFlags = kPropertyFlag_Visible) override;
                virtual PropertyFlags JsonSetProperties(const rapidjson::Value& input,
                                                        PropertyFlags propertyFlags = kPropertyFlag_All) override;
            };
        }
    }
}