#include "ScriptManager.hpp"
#include <home-database/Database.hpp>

namespace server
{
    namespace scripting
    {
        WeakRef<ScriptManager> instanceScriptManager;

        ScriptManager::ScriptManager(const boost::container::vector<Ref<ScriptProvider>>& providerList)
            : providerList(providerList)
        {
        }
        ScriptManager::~ScriptManager()
        {
        }
        Ref<ScriptManager> ScriptManager::Create(const boost::container::vector<Ref<ScriptProvider>>& providerList)
        {
            if (!instanceScriptManager.expired())
                return Ref<ScriptManager>(instanceScriptManager);

            Ref<ScriptManager> scriptManager = boost::make_shared<ScriptManager>(providerList);
            instanceScriptManager = scriptManager;
            if (scriptManager == nullptr)
                return nullptr;

            try
            {
                Ref<Database> database = Database::GetInstance();
                assert(database != nullptr);

                // Load script sources
                database->LoadScriptSources(boost::bind(
                    &ScriptManager::LoadScriptSource, scriptManager, boost::placeholders::_1, boost::placeholders::_2,
                    boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5));
            }
            catch (std::exception)
            {
                return nullptr;
            }

            return scriptManager;
        }
        Ref<ScriptManager> ScriptManager::GetInstance()
        {
            return Ref<ScriptManager>(instanceScriptManager);
        }

        bool ScriptManager::LoadScriptSource(identifier_t id, const std::string& language, const std::string& name,
                                             const std::string& usage, const std::string_view& content)
        {
            ScriptLanguage lang = ParseScriptLanguage(language);

            // Find provider
            const boost::container::vector<Ref<ScriptProvider>>::const_iterator it = boost::find_if(
                providerList, [&](Ref<ScriptProvider> provider) -> bool { return provider->GetLanguage() == lang; });

            if (it == providerList.end())
                return false;

            Ref<ScriptProvider> provider = *it;
            assert(provider != nullptr);

            // Create script source
            Ref<ScriptSource> scriptSource = provider->CreateScriptSource(id, name, ParseScriptUsage(usage), content);

            // Add script source
            if (scriptSource != nullptr)
                scriptSourceList[id] = scriptSource;

            return true;
        }

        Ref<ScriptSource> ScriptManager::AddScriptSource(ScriptLanguage language, const std::string& name,
                                                         ScriptUsage usage)
        {
            boost::lock_guard lock(mutex);

            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Find provider
            const boost::container::vector<Ref<ScriptProvider>>::const_iterator it =
                boost::find_if(providerList, [&](Ref<ScriptProvider> provider) -> bool {
                    return provider->GetLanguage() == language;
                });

            if (it == providerList.end())
                return nullptr;

            Ref<ScriptProvider> provider = *it;
            assert(provider != nullptr);

            // Reserve script source
            identifier_t id = database->ReserveScriptSource();
            if (id == 0)
                return nullptr;

            const std::string_view empty = std::string_view("", 0);

            // Update database
            if (!database->UpdateScriptSource(id, StringifyScriptLanguage(language), name, StringifyScriptUsage(usage),
                                              empty))
                return nullptr;

            // Create new script source
            Ref<ScriptSource> scriptSource = provider->CreateScriptSource(0, name, usage, empty);

            // Add script source
            if (scriptSource != nullptr)
                scriptSourceList[scriptSource->GetID()] = scriptSource;
            else
            {
                database->RemoveScriptSource(id);
                return nullptr;
            }

            return scriptSource;
        }

        Ref<ScriptSource> ScriptManager::GetScriptSource(identifier_t sourceID)
        {
            boost::lock_guard lock(mutex);

            const boost::unordered::unordered_map<identifier_t, Ref<ScriptSource>>::const_iterator it =
                scriptSourceList.find(sourceID);
            if (it == scriptSourceList.end())
                return nullptr;

            return it->second;
        }

        bool ScriptManager::RemoveScriptSource(identifier_t id)
        {
            boost::lock_guard lock(mutex);

            if (scriptSourceList.erase(id))
            {
                Ref<Database> database = Database::GetInstance();
                assert(database != nullptr);

                database->RemoveScriptSource(id);

                return true;
            }
            else
                return false;
        }

        Ref<Script> ScriptManager::CreateDeviceScript(identifier_t id)
        {
            LOG_CODE_MISSING("Create device script");
            return nullptr;
        }

        void ScriptManager::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            boost::lock_guard lock(mutex);

            // ScriptSources
            rapidjson::Value scriptSourceListJson = rapidjson::Value(rapidjson::kArrayType);

            for (auto& [id, scriptSource] : scriptSourceList)
            {
                assert(scriptSource != nullptr);

                rapidjson::Value scriptSourceJson = rapidjson::Value(rapidjson::kObjectType);

                scriptSource->JsonGet(scriptSourceJson, allocator);

                scriptSourceListJson.PushBack(scriptSourceJson, allocator);
            }

            output.AddMember("scriptsources", scriptSourceListJson, allocator);
        }
    }
}