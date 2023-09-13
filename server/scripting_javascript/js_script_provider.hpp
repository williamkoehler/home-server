#pragma once
#include "common.hpp"
#include <scripting/script_manager.hpp>

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

                virtual bool IsDynamic() override
                {
                    // JS script sources can be created dynamically
                    return true;
                }

                virtual boost::container::vector<StaticScriptSource> GetStaticScriptSources() override;

                /// @brief Create javascript script source
                ///
                /// @param id Script source id
                /// @param name Script source name
                /// @param usage Script usage
                /// @param data Source code
                /// @return Javascript script source
                virtual Ref<ScriptSource> CreateScriptSource(identifier_t id, const std::string& name,
                                                             const std::string_view& data) override;
            };
        }
    }
}