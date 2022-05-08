#pragma once
#include "ScriptSource.hpp"
#include "common.hpp"

namespace server
{
    namespace scripting
    {
        class ScriptProvider
        {
          public:
            virtual std::string GetName() = 0;
            virtual ScriptLanguage GetLanguage() = 0;

            virtual Ref<ScriptSource> CreateScriptSource(identifier_t id, const std::string& name, ScriptUsage usage,
                                                         const std::string_view& data) = 0;
        };

        class ScriptManager
        {
          private:
            boost::mutex mutex;

            boost::container::vector<Ref<ScriptProvider>> providerList;
            boost::unordered::unordered_map<identifier_t, Ref<ScriptSource>> scriptSourceList;

            // Database
            bool LoadScriptSource(identifier_t id, const std::string& language, const std::string& name,
                                  const std::string& usage, const std::string_view& content);

          public:
            ScriptManager(const boost::container::vector<Ref<ScriptProvider>>& providerList);
            virtual ~ScriptManager();
            static Ref<ScriptManager> Create(const boost::container::vector<Ref<ScriptProvider>>& providerList);
            static Ref<ScriptManager> GetInstance();

            //! Script Source

            Ref<ScriptSource> AddScriptSource(ScriptLanguage language, const std::string& name, ScriptUsage usage);

            inline size_t GetScriptSourceCount()
            {
                boost::lock_guard lock(mutex);
                return scriptSourceList.size();
            }

            Ref<ScriptSource> GetScriptSource(identifier_t id);

            bool RemoveScriptSource(identifier_t id);

            //! Script

            Ref<Script> CreateDeviceScript(identifier_t id, Ref<View> view);

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
        };
    }
}