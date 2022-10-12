#include "Core.hpp"
#include <home-scripting-javascript/JSScriptProvider.hpp>
#include <home-scripting-native/NativeScriptProvider.hpp>

namespace server
{
    WeakRef<Core> instanceCore;

    std::string GetServerName()
    {
        return Ref<Core>(instanceCore)->GetName();
    }

    Core::Core()
    {
    }
    Core::~Core()
    {
        home = nullptr;
        scriptManager = nullptr;
        userManager = nullptr;
        networkManager = nullptr;
        database = nullptr;

        if (worker != nullptr)
            worker->Stop();

        worker = nullptr;

        LOG_INFO("Terminating core server");
    }
    Ref<Core> Core::Create()
    {
        if (!instanceCore.expired())
            return Ref<Core>(instanceCore);

        Ref<Core> core = boost::make_shared<Core>();
        instanceCore = core;

        if (core != nullptr)
        {
            LOG_INFO("Initializing core server");

            CoreConfig config;
            if (!core->Load(config))
            {
                LOG_ERROR("Load configurations.");
                return nullptr;
            }

            LOG_INFO("This server is called '{0}'", core->name);

            // Initialize worker
            core->worker = Worker::Create();
            if (core->worker == nullptr)
            {
                LOG_ERROR("Initialize worker.");
                return nullptr;
            }

            // Initialize database
            core->database = Database::Create(config.database.type, config.database.location, config.database.username,
                                              config.database.password);
            if (core->database == nullptr)
            {
                LOG_ERROR("Initialize database.");
                return nullptr;
            }

            // Initialize default script provider
            boost::container::vector<Ref<scripting::ScriptProvider>> scriptProviderList = {
                // NativeScript
                scripting::native::NativeScriptProvider::Create(
                    boost::filesystem::absolute(config.scripting.nativeScript.source, config::GetScriptDirectory())
                        .string()),

                // JavaScript
                scripting::javascript::JSScriptProvider::Create(),
            };
            for (Ref<scripting::ScriptProvider> scriptProvider : scriptProviderList)
            {
                if (scriptProvider == nullptr)
                {
                    LOG_ERROR("Initialize script provider.");
                    return nullptr;
                }
            }

            // Initialize scripting
            core->scriptManager = scripting::ScriptManager::Create(scriptProviderList);
            if (core->scriptManager == nullptr)
            {
                LOG_ERROR("Initialize script manager.");
                return nullptr;
            }

            // Initialize home
            core->home = main::Home::Create();
            if (core->home == nullptr)
            {
                LOG_ERROR("Initialize home.");
                return nullptr;
            }

            core->userManager = users::UserManager::Create(core->name);
            if (core->userManager == nullptr)
            {
                LOG_ERROR("Initialize user manager.");
                return nullptr;
            }

            // Initialize networking
            core->networkManager = networking::NetworkManager::Create(core->address, core->port, core->externalURL);
            if (core->networkManager == nullptr)
            {
                LOG_ERROR("Intialize network manager.");
                return nullptr;
            }
        }

        LOG_FLUSH();

        return core;
    }

    Ref<Core> Core::GetInstance()
    {
        return Ref<Core>(instanceCore);
    }

    void Core::Run()
    {
        // Run worker
        worker->Run();
    }

    // Shutdown
    void Core::Shutdown()
    {
        worker->Stop();
    }

    bool Core::Load(CoreConfig& config)
    {
        std::string configFile = (config::GetConfigurationDirectory() / CONFIGURATION_FILE_NAME).string();

        LOG_INFO("Loading configurations from '{0}'", configFile);

        std::ifstream file = std::ifstream(configFile);
        if (!file.is_open())
        {
            LOG_ERROR("Missing config file '{0}'", configFile);
            return false;
        }

        char buffer[RAPIDJSON_BUFFER_SIZE_SMALL];
        rapidjson::IStreamWrapper stream = rapidjson::IStreamWrapper(file, buffer, sizeof(buffer));

        rapidjson::Document document;
        if (document.ParseStream(stream).HasParseError() || !document.IsObject())
        {
            LOG_ERROR("Parse config file '{0}'", configFile);
            return false;
        }

        // Load server name
        {
            rapidjson::Value::MemberIterator nameIt = document.FindMember("name");
            if (nameIt != document.MemberEnd() && nameIt->value.IsString())
                name.assign(nameIt->value.GetString(), nameIt->value.GetStringLength());
            else
            {
                name = "My Home Automation server v" SERVER_VERSION;
                LOG_WARNING("Missing 'name'. Name will be set to '{0}'.", name);
            }
        }

        // Load database
        {
            rapidjson::Value::MemberIterator databaseIt = document.FindMember("database");
            if (databaseIt != document.MemberEnd() && databaseIt->value.IsObject())
            {
                rapidjson::Value& databaseJson = databaseIt->value;

                // Load type
                rapidjson::Value::MemberIterator typeIt = databaseJson.FindMember("type");
                if (typeIt != databaseJson.MemberEnd() && typeIt->value.IsString())
                {
                    std::string database = std::string(typeIt->value.GetString(), typeIt->value.GetStringLength());
                    config.database.type = ParseDatabaseType(database);
                    if (config.database.type == DatabaseType::kUnknownDatabaseType)
                    {
                        config.database.type = DatabaseType::kSQLiteDatabaseType;
                        LOG_WARNING("Invalid database type '{0}'. Database type will be set to default 'sqlite'.",
                                    database);
                    }
                }
                else
                {
                    config.database.type = DatabaseType::kSQLiteDatabaseType;
                    LOG_WARNING("Missing 'database.type'. Database type will be set to '{0}'.",
                                StringifyDatabaseType(config.database.type));
                }

                // Load location
                rapidjson::Value::MemberIterator locationIt = databaseJson.FindMember("location");
                if (locationIt != databaseJson.MemberEnd() && locationIt->value.IsString())
                    config.database.location =
                        std::string(locationIt->value.GetString(), locationIt->value.GetStringLength());
                else
                    config.database.location = "";

                // Load username
                rapidjson::Value::MemberIterator usernameIt = databaseJson.FindMember("username");
                if (usernameIt != databaseJson.MemberEnd() && usernameIt->value.IsString())
                    config.database.username =
                        std::string(usernameIt->value.GetString(), usernameIt->value.GetStringLength());
                else
                    config.database.username = "";

                // Load password
                rapidjson::Value::MemberIterator passwordIt = databaseJson.FindMember("password");
                if (passwordIt != databaseJson.MemberEnd() && passwordIt->value.IsString())
                    config.database.password =
                        std::string(passwordIt->value.GetString(), passwordIt->value.GetStringLength());
                else
                    config.database.password = "";
            }
            else
            {
                LOG_WARNING("Missing 'database' object.");
            }
        }

        // Load networking
        {
            rapidjson::Value::MemberIterator networkingIt = document.FindMember("networking");
            if (networkingIt != document.MemberEnd() && networkingIt->value.IsObject())
            {
                rapidjson::Value& networkingJson = networkingIt->value;

                // Load web server address
                rapidjson::Value::MemberIterator addressIt = networkingJson.FindMember("address");
                if (addressIt != networkingJson.MemberEnd() && addressIt->value.IsString())
                    address = std::string(addressIt->value.GetString(), addressIt->value.GetStringLength());
                else
                {
                    address = "0.0.0.0";
                    LOG_WARNING("Missing 'networking.address'. Address will be set to default '0.0.0.0'.");
                }

                // Load web server port
                rapidjson::Value::MemberIterator portIt = networkingJson.FindMember("port");
                if (portIt != networkingJson.MemberEnd() && portIt->value.IsUint())
                    port = portIt->value.GetUint();
                else
                {
                    port = 443;
                    LOG_WARNING("Missing 'networking.port'. Port will be set to default '{0}'.", port);
                }

                LOG_INFO("Server address is {0}:{1}", address, port);

                // Load web server external address
                rapidjson::Value::MemberIterator externalURLIt = networkingJson.FindMember("external-url");
                if (externalURLIt != networkingJson.MemberEnd() && externalURLIt->value.IsString())
                    externalURL = std::string(externalURLIt->value.GetString(), externalURLIt->value.GetStringLength());
                else
                {
                    externalURL = "127.0.0.1";
                    LOG_WARNING("Missing 'networking.external-url'. External Url will be set to default '{0}'.",
                                externalURL);
                }
            }
            else
            {
                LOG_WARNING("Missing 'networking' object.");
            }
        }

        // Load scripting config
        {
            rapidjson::Value::MemberIterator scriptingIt = document.FindMember("scripting");
            if (scriptingIt != document.MemberEnd() && scriptingIt->value.IsObject())
            {
                rapidjson::Value& scriptingJson = scriptingIt->value;

                // Load native script config
                rapidjson::Value::MemberIterator nativeScriptIt = scriptingJson.FindMember("native-script");
                if (scriptingIt != scriptingJson.MemberEnd() && scriptingIt->value.IsObject())
                {
                    rapidjson::Value& nativeScriptJson = nativeScriptIt->value;

                    // Load native script source
                    rapidjson::Value::MemberIterator sourceIt = nativeScriptJson.FindMember("source");
                    if (sourceIt != nativeScriptJson.MemberEnd() && sourceIt->value.IsString())
                        config.scripting.nativeScript.source =
                            std::string(sourceIt->value.GetString(), sourceIt->value.GetStringLength());
                    else
                    {
                        config.scripting.nativeScript.source = "";
                        LOG_WARNING("Missing 'scripting.native-script.source'. Native script directory will be set to "
                                    "default '{0}'.",
                                    config::GetScriptDirectory().string());
                    }
                }
                else
                {
                    LOG_WARNING("Missing 'scripting.native-script' object.");
                }
            }
            else
            {
                LOG_WARNING("Missing 'scripting' object.");
            }
        }

        return true;
    }
}