#pragma once
#include "common.hpp"
#include <home-scripting/ScriptManager.hpp>

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            /// @brief Javascript Script Provider
            ///
            class JSScriptProvider : public ScriptProvider
            {
              public:
                JSScriptProvider();
                virtual ~JSScriptProvider();
                static Ref<ScriptProvider> Create();

                virtual std::string GetName() override
                {
                    return "JavaScript Script Source Provider";
                }
                virtual ScriptLanguage GetLanguage() override
                {
                    return ScriptLanguage::kJSScriptLanguage;
                }

                /// @brief Create javascript script source
                ///
                /// @param id Script source id
                /// @param name Script source name
                /// @param usage Script usage
                /// @param data Source code
                /// @return Javascript script source
                virtual Ref<ScriptSource> CreateScriptSource(identifier_t id, const std::string& name,
                                                             ScriptUsage usage, const std::string_view& data) override;
            };
        }
    }
}