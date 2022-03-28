#pragma once
#include "common.hpp"
#include <home-scripting/ScriptSource.hpp>

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            class JSScriptSource : public ScriptSource
            {
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

                /// @brief Create javascript script
                /// 
                /// @param view Sender view
                /// @return Script or null in case of an error
                virtual Ref<Script> CreateScript(Ref<View> view) override;
            };
        }
    }
}