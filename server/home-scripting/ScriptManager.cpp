#include "ScriptManager.hpp"
#include <home-database/Database.hpp>

namespace server
{
    namespace scripting
    {
        WeakRef<ScriptManager> instanceScriptManager;
        WeakRef<HomeView> instanceHomeView;

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

            // Verify script providers
            for (const Ref<ScriptProvider>& provider : providerList)
            {
                if (provider == nullptr)
                {
                    LOG_ERROR("Invalid script provider.");
                    return nullptr;
                }
            }

            try
            {
                Ref<Database> database = Database::GetInstance();
                assert(database != nullptr);

                // Load script sources
                database->LoadScriptSources(boost::bind(
                    &ScriptManager::LoadScriptSource, scriptManager, boost::placeholders::_1, boost::placeholders::_2,
                    boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5));

                // Load static script sources
                for (const Ref<ScriptProvider>& provider : scriptManager->providerList)
                {
                    // Get static script sources
                    boost::container::vector<StaticScriptSource> staticScriptSources =
                        provider->GetStaticScriptSources();

                    // Create static script sources
                    for (StaticScriptSource& staticScriptSource : staticScriptSources)
                    {
                        scriptManager->AddScriptSource(provider->GetLanguage(), staticScriptSource.name);
                    }
                }
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

        void ScriptManager::SetHomeView(const Ref<HomeView>& homeView)
        {
            assert(homeView != nullptr);
            instanceHomeView = homeView;
        }
        Ref<HomeView> ScriptManager::GetHomeView()
        {
            return Ref<HomeView>(instanceHomeView);
        }

        bool ScriptManager::LoadScriptSource(identifier_t id, const std::string& language, const std::string& name,
                                             const std::string_view& config, const std::string_view& content)
        {
            (void)config;

            const ScriptLanguage lang = ParseScriptLanguage(language);

            // Find provider
            const boost::container::vector<Ref<ScriptProvider>>::const_iterator it = boost::find_if(
                providerList,
                [&](const Ref<ScriptProvider>& provider) -> bool { return provider->GetLanguage() == lang; });

            if (it == providerList.end())
                return false;

            const Ref<ScriptProvider>& provider = *it;
            assert(provider != nullptr);

            // Create script source
            Ref<ScriptSource> scriptSource = provider->CreateScriptSource(id, name, content);

            // Add script source
            if (scriptSource != nullptr)
                scriptSourceList[id] = scriptSource;

            return true;
        }

        Ref<ScriptSource> ScriptManager::AddScriptSource(ScriptLanguage language, const std::string& name)
        {
            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Find provider
            const boost::container::vector<Ref<ScriptProvider>>::const_iterator it = boost::find_if(
                providerList,
                [&](const Ref<ScriptProvider>& provider) -> bool { return provider->GetLanguage() == language; });

            if (it == providerList.end())
                return nullptr;

            const Ref<ScriptProvider>& provider = *it;
            assert(provider != nullptr);

            // Reserve script source
            identifier_t id = database->ReserveScriptSource(StringifyScriptLanguage(language));
            if (id == 0)
                return nullptr;

            // Update database
            if (!database->UpdateScriptSource(id, name, std::string_view("", 0)))
                return nullptr;

            // Create new script source
            Ref<ScriptSource> scriptSource = provider->CreateScriptSource(id, name, std::string_view("", 0));

            // Add script source
            if (scriptSource != nullptr)
                scriptSourceList[id] = scriptSource;
            else
            {
                database->RemoveScriptSource(id);
                return nullptr;
            }

            return scriptSource;
        }

        Ref<ScriptSource> ScriptManager::GetScriptSource(identifier_t id)
        {
            const robin_hood::unordered_node_map<identifier_t, Ref<ScriptSource>>::const_iterator it =
                scriptSourceList.find(id);
            if (it == scriptSourceList.end())
                return nullptr;

            return it->second;
        }

        bool ScriptManager::RemoveScriptSource(identifier_t id)
        {
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

        Ref<Script> ScriptManager::CreateScript(identifier_t id, uint8_t flags, const Ref<View>& view)
        {
            const robin_hood::unordered_node_map<identifier_t, Ref<ScriptSource>>::const_iterator it =
                scriptSourceList.find(id);
            if (it == scriptSourceList.end())
                return nullptr;

            const Ref<ScriptSource>& scriptSource = it->second;

            // Verify flags
            // A bit is only set to 1 if a filter flag is not set in the script source flags
            if (~scriptSource->GetFlags() & flags)
                return nullptr;

            return scriptSource->CreateScript(view);
        }

        void ScriptManager::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) const
        {
            rapidjson::Value scriptSourceListJson = rapidjson::Value(rapidjson::kArrayType);

            for (const auto& [id, scriptSource] : scriptSourceList)
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