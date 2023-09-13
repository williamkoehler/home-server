#include "core.hpp"
#include <scripting_javascript/js_script_provider.hpp>
#include <scripting_native/native_script_provider.hpp>

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

            // Initialize core
            core->name = config.name;
            LOG_INFO("This server is called '{0}'", core->name);

            // Initialize worker
            {
                core->worker = Worker::Create();
                if (core->worker == nullptr)
                {
                    LOG_ERROR("Initialize worker.");
                    return nullptr;
                }
            }

            // Initialize database
            {
                core->database = Database::Create(config.database.type, config.database.location,
                                                  config.database.username, config.database.password);
                if (core->database == nullptr)
                {
                    LOG_ERROR("Initialize database.");
                    return nullptr;
                }
            }

            // Initialize api
            {
                // Initialize user manager
                core->userManager = api::UserManager::Create(core->name);
                if (core->userManager == nullptr)
                {
                    LOG_ERROR("Initialize user manager.");
                    return nullptr;
                }

                // Initialize networking
                core->networkManager = api::NetworkManager::Create(config.networking.address, config.networking.port,
                                                                   config.networking.externalURL);
                if (core->networkManager == nullptr)
                {
                    LOG_ERROR("Intialize network manager.");
                    return nullptr;
                }
            }

            // Initialize scripting
            {
                // Initialize default script provider
                boost::container::vector<Ref<scripting::ScriptProvider>> scriptProviderList = {
                    // NativeScript
                    scripting::native::NativeScriptProvider::Create(
                        boost::filesystem::absolute(config.scripting.nativeScript.source, config::GetScriptDirectory())
                            .string()),

                    // JavaScript
                    scripting::javascript::JSScriptProvider::Create(),
                };
                for (const Ref<scripting::ScriptProvider> &scriptProvider : scriptProviderList)
                {
                    if (scriptProvider == nullptr)
                    {
                        LOG_ERROR("Initialize script provider.");
                        return nullptr;
                    }
                }

                // Initialize script manager
                core->scriptManager = scripting::ScriptManager::Create(scriptProviderList);
                if (core->scriptManager == nullptr)
                {
                    LOG_ERROR("Initialize script manager.");
                    return nullptr;
                }
            }

            // Initialize home
            core->home = main::Home::Create();
            if (core->home == nullptr)
            {
                LOG_ERROR("Initialize home.");
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
                config.name.assign(nameIt->value.GetString(), nameIt->value.GetStringLength());
            else
            {
                config.name = "My Home Automation server v" SERVER_VERSION;
                LOG_WARNING("Missing 'name'. Name will be set to '{0}'.", config.name);
            }
        }

        // Load database
        {
            CoreConfig::DatabaseConfig& databaseConfig = config.database;

            rapidjson::Value::MemberIterator databaseIt = document.FindMember("database");
            if (databaseIt != document.MemberEnd() && databaseIt->value.IsObject())
            {
                rapidjson::Value& databaseJson = databaseIt->value;

                // Load type
                rapidjson::Value::MemberIterator typeIt = databaseJson.FindMember("type");
                if (typeIt != databaseJson.MemberEnd() && typeIt->value.IsString())
                {
                    std::string database = std::string(typeIt->value.GetString(), typeIt->value.GetStringLength());
                    databaseConfig.type = ParseDatabaseType(database);
                    if (databaseConfig.type == DatabaseType::kUnknownDatabaseType)
                    {
                        databaseConfig.type = DatabaseType::kSQLiteDatabaseType;
                        LOG_WARNING("Invalid database type '{0}'. Database type will be set to default 'sqlite'.",
                                    database);
                    }
                }
                else
                {
                    databaseConfig.type = DatabaseType::kSQLiteDatabaseType;
                    LOG_WARNING("Missing 'database.type'. Database type will be set to '{0}'.",
                                StringifyDatabaseType(databaseConfig.type));
                }

                // Load location
                rapidjson::Value::MemberIterator locationIt = databaseJson.FindMember("location");
                if (locationIt != databaseJson.MemberEnd() && locationIt->value.IsString())
                    databaseConfig.location =
                        std::string(locationIt->value.GetString(), locationIt->value.GetStringLength());
                else
                    databaseConfig.location = "";

                // Load username
                rapidjson::Value::MemberIterator usernameIt = databaseJson.FindMember("username");
                if (usernameIt != databaseJson.MemberEnd() && usernameIt->value.IsString())
                    config.database.username =
                        std::string(usernameIt->value.GetString(), usernameIt->value.GetStringLength());
                else
                    databaseConfig.username = "";

                // Load password
                rapidjson::Value::MemberIterator passwordIt = databaseJson.FindMember("password");
                if (passwordIt != databaseJson.MemberEnd() && passwordIt->value.IsString())
                    config.database.password =
                        std::string(passwordIt->value.GetString(), passwordIt->value.GetStringLength());
                else
                    databaseConfig.password = "";
            }
            else
            {
                LOG_WARNING("Missing 'database' object.");
            }
        }

        // Load networking
        {
            CoreConfig::NetworkingConfig& networkingConfig = config.networking;

            rapidjson::Value::MemberIterator networkingIt = document.FindMember("networking");
            if (networkingIt != document.MemberEnd() && networkingIt->value.IsObject())
            {
                rapidjson::Value& networkingJson = networkingIt->value;

                // Load web server address
                rapidjson::Value::MemberIterator addressIt = networkingJson.FindMember("address");
                if (addressIt != networkingJson.MemberEnd() && addressIt->value.IsString())
                    config.networking.address =
                        std::string(addressIt->value.GetString(), addressIt->value.GetStringLength());
                else
                {
                    networkingConfig.address = "0.0.0.0";
                    LOG_WARNING("Missing 'networking.address'. Address will be set to default '0.0.0.0'.");
                }

                // Load web server port
                rapidjson::Value::MemberIterator portIt = networkingJson.FindMember("port");
                if (portIt != networkingJson.MemberEnd() && portIt->value.IsUint())
                    networkingConfig.port = portIt->value.GetUint();
                else
                {
                    networkingConfig.port = 443;
                    LOG_WARNING("Missing 'networking.port'. Port will be set to default '{0}'.", networkingConfig.port);
                }

                LOG_INFO("Server address is {0}:{1}", networkingConfig.address, networkingConfig.port);

                // Load web server external address
                rapidjson::Value::MemberIterator externalURLIt = networkingJson.FindMember("external-url");
                if (externalURLIt != networkingJson.MemberEnd() && externalURLIt->value.IsString())
                    networkingConfig.externalURL =
                        std::string(externalURLIt->value.GetString(), externalURLIt->value.GetStringLength());
                else
                {
                    networkingConfig.externalURL = "127.0.0.1";
                    LOG_WARNING("Missing 'networking.external-url'. External Url will be set to default '{0}'.",
                                networkingConfig.externalURL);
                }
            }
            else
            {
                LOG_WARNING("Missing 'networking' object.");
            }
        }

        // Load scripting config
        {
            CoreConfig::ScriptingConfig& scriptingConfig = config.scripting;

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
                        scriptingConfig.nativeScript.source =
                            std::string(sourceIt->value.GetString(), sourceIt->value.GetStringLength());
                    else
                    {
                        scriptingConfig.nativeScript.source = "";
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