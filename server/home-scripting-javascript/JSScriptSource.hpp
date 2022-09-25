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
                JSScriptSource(identifier_t id, const std::string& name, ScriptUsage usage,
                               const std::string_view& data);
                virtual ~JSScriptSource();
                static Ref<JSScriptSource> Create(identifier_t id, const std::string& name, ScriptUsage usage,
                                                  const std::string_view& data);

                /// @brief Get script language
                ///
                /// @return JavaScript
                virtual ScriptLanguage GetLanguage() override
                {
                    return ScriptLanguage::kJSScriptLanguage;
                }

                virtual bool SetContent(const std::string_view& v) override;

                /// @brief Create javascript script
                ///
                /// @param view Sender view
                /// @return Script or null in case of an error
                virtual Ref<Script> CreateScript(Ref<View> view) override;

                /// @brief Remove deleted scripts
                /// @note This method is automatically called when a script is deleted
                ///
                void CleanScripts();
            };
        }
    }
}