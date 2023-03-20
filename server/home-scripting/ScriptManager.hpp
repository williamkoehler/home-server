#pragma once
#include "ScriptSource.hpp"
#include "common.hpp"

namespace server
{
    namespace scripting
    {
        class ScriptManager;
        class HomeView;

        struct StaticScriptSource
        {
            const std::string name;
            const ScriptUsage usage;
        };

        class ScriptProvider
        {
          public:
            /// @brief Get script provider name
            ///
            /// @return Script provider name
            virtual std::string GetName() = 0;
            virtual ScriptLanguage GetLanguage() = 0;

            /// @brief Can the script provider create scripts sources dynamically from source code.
            /// For example JavaScript that only loads from source code.
            ///
            /// @return true Can create script source from source code
            /// @return false Can only create static script sources
            virtual bool IsDynamic() = 0;

            /// @brief Get static script sources
            ///
            /// @return Static script sources
            virtual boost::container::vector<StaticScriptSource> GetStaticScriptSources() = 0;

            /// @brief Create script source
            ///
            /// @param id Script source id
            /// @param name Script source name
            /// @param usage Script usage
            /// @param content Source code
            /// @return Script Source
            virtual Ref<ScriptSource> CreateScriptSource(identifier_t id, const std::string& name, ScriptUsage usage,
                                                         const std::string_view& content) = 0;
        };

        class ScriptManager
        {
          private:
            const Ref<HomeView> homeView;

            boost::container::vector<Ref<ScriptProvider>> providerList;
            robin_hood::unordered_node_map<identifier_t, Ref<ScriptSource>> scriptSourceList;

            // Database
            bool LoadScriptSource(identifier_t id, const std::string& language, const std::string& name,
                                  const std::string& usage, const std::string_view& content);

          public:
            ScriptManager(const boost::container::vector<Ref<ScriptProvider>>& providerList);
            virtual ~ScriptManager();
            static Ref<ScriptManager> Create(const boost::container::vector<Ref<ScriptProvider>>& providerList);
            static Ref<ScriptManager> GetInstance();

            // Get singleton views
            static void SetHomeView(const Ref<HomeView>& homeView);
            static Ref<HomeView> GetHomeView();

            //! Script Source

            Ref<ScriptSource> AddScriptSource(ScriptLanguage language, const std::string& name, ScriptUsage usage);

            inline size_t GetScriptSourceCount()
            {
                return scriptSourceList.size();
            }

            Ref<ScriptSource> GetScriptSource(identifier_t id);

            bool RemoveScriptSource(identifier_t id);

            //! Script

            Ref<Script> CreateDeviceScript(identifier_t id, const Ref<View>& view);
            Ref<Script> CreateServiceScript(identifier_t id, const Ref<View>& view);

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
        };
    }
}