#pragma once
#include "common.hpp"
#include <boost/dll/shared_library.hpp>
#include <scripting/script_manager.hpp>
#include <scripting_sdk/library_information.hpp>

namespace server
{
    namespace scripting
    {
        namespace native
        {
            /// @brief Native Script Provider
            ///
            class NativeScriptProvider : public ScriptProvider
            {
              private:
                /// @brief Directory path where native scripts can be found
                ///
                std::string path;

                boost::container::vector<Ref<boost::dll::shared_library>> libraryList;
                robin_hood::unordered_node_map<std::string, sdk::ScriptInformation> scriptList;

              public:
                NativeScriptProvider(const std::string& path);
                virtual ~NativeScriptProvider();
                static Ref<ScriptProvider> Create(const std::string& path);

                virtual std::string GetName() override
                {
                    return "Native Script Source Provider";
                }
                virtual ScriptLanguage GetLanguage() override
                {
                    return ScriptLanguage::kNativeScriptLanguage;
                }

                virtual bool IsDynamic() override
                {
                    // Only static script sources
                    return false;
                }

                virtual boost::container::vector<StaticScriptSource> GetStaticScriptSources() override;

                /// @brief Does nothing. Every native script is static
                ///
                virtual Ref<ScriptSource> CreateScriptSource(identifier_t id, const std::string& name,
                                                             const std::string_view& content) override;
            };
        }
    }
}