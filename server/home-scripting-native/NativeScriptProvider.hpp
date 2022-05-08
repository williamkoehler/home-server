#pragma once
#include "common.hpp"
#include "robin-hood/robin_hood.h"
#include <boost/dll/shared_library.hpp>
#include <home-scripting/ScriptManager.hpp>

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
                boost::mutex mutex;

                /// @brief Directory path where native scripts can be found
                ///
                std::string path;

                robin_hood::unordered_node_map<std::string, Ref<boost::dll::shared_library>> libraryList;

                void LoadLibraries();

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