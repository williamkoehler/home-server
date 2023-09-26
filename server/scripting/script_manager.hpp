#pragma once
#include "common.hpp"
#include "script_source.hpp"
#include <api/message.hpp>
#include <scripting_sdk/view/main/home_view.hpp>

namespace server
{
    namespace scripting
    {
        class ScriptManager;

        struct StaticScriptSource
        {
            const std::string name;
            const uint8_t flags;
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
            virtual Ref<ScriptSource> CreateScriptSource(identifier_t id, const std::string& name,
                                                         const std::string_view& content) = 0;
        };

        class ScriptManager
        {
          private:
            const Ref<sdk::HomeView> homeView;

            boost::container::vector<Ref<ScriptProvider>> providerList;
            robin_hood::unordered_node_map<identifier_t, Ref<ScriptSource>> scriptSourceList;

            // Database
            bool LoadScriptSource(identifier_t id, const std::string& language, const std::string& name,
                                  const std::string_view& config, const std::string_view& content);

          public:
            ScriptManager(const boost::container::vector<Ref<ScriptProvider>>& providerList);
            virtual ~ScriptManager();
            static Ref<ScriptManager> Create(const boost::container::vector<Ref<ScriptProvider>>& providerList);
            static Ref<ScriptManager> GetInstance();

            // Get singleton views
            static void SetHomeView(const Ref<sdk::HomeView>& homeView);
            static Ref<sdk::HomeView> GetHomeView();

            //! Script Source

            Ref<ScriptSource> AddScriptSource(ScriptLanguage language, const std::string& name);

            inline size_t GetScriptSourceCount()
            {
                return scriptSourceList.size();
            }

            Ref<ScriptSource> GetScriptSource(identifier_t id);

            bool RemoveScriptSource(identifier_t id);

            Ref<Script> CreateScript(identifier_t id, uint8_t flags, const Ref<sdk::View>& view);

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) const;

            //! WebSocket Api
            static void WebSocketProcessGetScriptSourcesMessage(const Ref<api::User>& user,
                                                                const api::ApiRequestMessage& request,
                                                                api::ApiResponseMessage& response,
                                                                const Ref<api::WebSocketSession>& session);

            static void WebSocketProcessAddScriptSourceMessage(const Ref<api::User>& user,
                                                               const api::ApiRequestMessage& request,
                                                               api::ApiResponseMessage& response,
                                                               const Ref<api::WebSocketSession>& session);
            static void WebSocketProcessRemoveScriptSourceMessage(const Ref<api::User>& user,
                                                                  const api::ApiRequestMessage& request,
                                                                  api::ApiResponseMessage& response,
                                                                  const Ref<api::WebSocketSession>& session);

            static void WebSocketProcessGetScriptSourceMessage(const Ref<api::User>& user,
                                                               const api::ApiRequestMessage& request,
                                                               api::ApiResponseMessage& response,
                                                               const Ref<api::WebSocketSession>& session);
            static void WebSocketProcessSetScriptSourceMessage(const Ref<api::User>& user,
                                                               const api::ApiRequestMessage& request,
                                                               api::ApiResponseMessage& response,
                                                               const Ref<api::WebSocketSession>& session);

            static void WebSocketProcessGetScriptSourceContentMessage(const Ref<api::User>& user,
                                                                      const api::ApiRequestMessage& request,
                                                                      api::ApiResponseMessage& response,
                                                                      const Ref<api::WebSocketSession>& session);
            static void WebSocketProcessSetScriptSourceContentMessage(const Ref<api::User>& user,
                                                                      const api::ApiRequestMessage& request,
                                                                      api::ApiResponseMessage& response,
                                                                      const Ref<api::WebSocketSession>& session);
        };
    }
}