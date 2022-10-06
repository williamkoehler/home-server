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
            class NativeScriptImpl;

            class NativeScriptSource : public ScriptSource
            {
              private:
                CreateScriptCallback<> callback;

              public:
                NativeScriptSource(identifier_t id, const std::string& name, ScriptUsage usage,
                                   CreateScriptCallback<> callback);
                virtual ~NativeScriptSource();
                static Ref<NativeScriptSource> Create(identifier_t id, const std::string& name, ScriptUsage usage,
                                                      CreateScriptCallback<> callback);

                /// @brief Get script language
                ///
                /// @return Native script
                virtual ScriptLanguage GetLanguage() override
                {
                    return ScriptLanguage::kNativeScriptLanguage;
                }

                /// @brief Prevent content from being overwritten, since it does not serve any purpose
                ///
                /// @param data
                /// @return Always false
                virtual bool SetContent(const std::string_view& data) override
                {
                    return false;
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
                virtual Ref<Script> CreateScript(Ref<View> view) override;
            };
        }
    }
}