#pragma once
#include "NativeScript.hpp"
#include "common.hpp"
#include <home-scripting/ScriptSource.hpp>

namespace server
{
    namespace scripting
    {
        namespace native
        {
            class NativeScriptSource : public ScriptSource
            {
              private:
                uint8_t flags;

                CreateScriptCallback<> callback;

              public:
                NativeScriptSource(identifier_t id, const std::string& name, uint8_t flags,
                                   CreateScriptCallback<> callback);
                virtual ~NativeScriptSource();
                static Ref<NativeScriptSource> Create(identifier_t id, const std::string& name, uint8_t flags,
                                                      CreateScriptCallback<> callback);

                /// @brief Get script flags
                ///
                /// @return uint8_t Script flags
                virtual uint8_t GetFlags() const override
                {
                    return flags;
                }

                /// @brief Get script language
                ///
                /// @return Native script
                virtual ScriptLanguage GetLanguage() override
                {
                    return ScriptLanguage::kNativeScriptLanguage;
                }

                /// @brief Prevent content from returning anything, as it does not serve any purpose in a native script
                ///
                /// @return std::string Always an empty string
                virtual std::string GetContent() const override
                {
                    return std::string();
                }

                /// @brief Prevent content from being overwritten, ad it does not serve any purpose in a native script
                ///
                /// @param data
                virtual void SetContent(const std::string_view& v) override
                {
                    (void)v;
                }

                /// @brief Get create callback
                ///
                /// @return Callback
                inline CreateScriptCallback<> GetCreateCallback() const
                {
                    return callback;
                }

                /// @brief Create native script
                ///
                /// @param view Sender view
                /// @return Script or null in case of an error
                virtual Ref<Script> CreateScript(const Ref<View>& view) override;

                virtual void JsonGetConfig(rapidjson::Value& output,
                                           rapidjson::Document::AllocatorType& allocator) override;
                virtual bool JsonSetConfig(const rapidjson::Value& input) override;
            };
        }
    }
}