#pragma once
#include "common.hpp"
#include <home-scripting/ScriptSource.hpp>

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            class JSScript;

            class JSScriptSource : public ScriptSource
            {
              private:
                boost::container::vector<WeakRef<JSScript>> scriptList;

              public:
                JSScriptSource(identifier_t id, const std::string& name, const std::string_view& content);
                virtual ~JSScriptSource();
                static Ref<JSScriptSource> Create(identifier_t id, const std::string& name,
                                                  const std::string_view& content);

                /// @brief Get default script flags
                ///
                /// @return uint8_t Script flags
                virtual uint8_t GetFlags() const override
                {
                    return ScriptFlags::kScriptFlag_DeviceSupport | ScriptFlags::kScriptFlag_RoomSupport;
                }

                /// @brief Get script language
                ///
                /// @return JavaScript
                virtual ScriptLanguage GetLanguage() override
                {
                    return ScriptLanguage::kJSScriptLanguage;
                }

                virtual void SetContent(const std::string_view& v) override;

                /// @brief Create javascript script
                ///
                /// @param view Sender view
                /// @return Script or null in case of an error
                virtual Ref<Script> CreateScript(const Ref<View>& view) override;

                /// @brief Remove deleted scripts
                /// @note This method is automatically called when a script is deleted
                ///
                void CleanScripts();

                virtual void JsonGetConfig(rapidjson::Value &output, rapidjson::Document::AllocatorType &allocator) override;
                virtual bool JsonSetConfig(const rapidjson::Value &input) override;
            };
        }
    }
}