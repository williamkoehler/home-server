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

        worker->Stop();

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

            if (!core->Load())
            {
                LOG_ERROR("Load configurations.");
                return nullptr;
            }

            LOG_INFO("This server is called '{0}'", core->name);

            // Init worker
            core->worker = threading::Worker::Create("Core", core->threadCount);
            if (core->worker == nullptr)
            {
                LOG_ERROR("Initialize core worker.");
                return nullptr;
            }

            // Initialize database
            core->database = Database::Create(DatabaseType::kSQLiteDatabaseType,
                                              (config::GetStateDirectory() / "home.sqlite3").string());
            if (core->database == nullptr)
            {
                LOG_ERROR("Initialize database.");
                return nullptr;
            }

            // Initialize default script provider
            boost::container::vector<Ref<scripting::ScriptProvider>> scriptProviderList = {
                scripting::native::NativeScriptProvider::Create(config::GetScriptDirectory().string()), // NativeScript
                scripting::javascript::JSScriptProvider::Create(),                                      // JavaScript
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
            core->networkManager =
                networking::NetworkManager::Create(core->worker, core->address, core->port, core->externalURL);
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
        worker->Start(true);
    }

    // Shutdown
    void Core::Shutdown()
    {
        worker->Stop();
    }

    bool Core::Load()
    {
        std::string configFile = (config::GetConfigurationDirectory() / CONFIGURATION_FILE_NAME).string();

        LOG_INFO("Loading configurations from '{0}'", configFile);

        std::ifstream file = std::ifstream(configFile);
        if (!file.is_open())
        {
            LOG_ERROR("Open config file '{0}'", configFile);
            return false;
        }

        boost::lock_guard lock(mutex);

        char buffer[RAPIDJSON_BUFFER_SIZE_SMALL];
        rapidjson::IStreamWrapper stream = rapidjson::IStreamWrapper(file, buffer, sizeof(buffer));

        rapidjson::Document document;
        if (document.ParseStream(stream).HasParseError() || !document.IsObject())
        {
            LOG_ERROR("Parse config file '{0}'", configFile);
            return false;
        }

        // Load server name
        rapidjson::Value::MemberIterator nameIt = document.FindMember("name");
        if (nameIt != document.MemberEnd() && nameIt->value.IsString())
            name.assign(nameIt->value.GetString(), nameIt->value.GetStringLength());
        else
        {
            name = "My Home Automation server v" SERVER_VERSION;
            LOG_WARNING("Missing 'name'. Name will be set to '{0}'.", name);
        }

        // Load web server address
        rapidjson::Value::MemberIterator addressIt = document.FindMember("address");
        if (addressIt != document.MemberEnd() && addressIt->value.IsString())
            address = std::string(addressIt->value.GetString(), addressIt->value.GetStringLength());
        else
        {
            address = "0.0.0.0";
            LOG_WARNING("Missing 'address'. Address will be set to default '0.0.0.0'.");
        }

        // Load web server port
        rapidjson::Value::MemberIterator portIt = document.FindMember("port");
        if (portIt != document.MemberEnd() && portIt->value.IsUint())
            port = portIt->value.GetUint();
        else
        {
            port = 443;
            LOG_WARNING("Missing 'port'. Port will be set to default '{0}'.", port);
        }

        LOG_INFO("Server address is {0}:{1}", address, port);

        // Load web server external address
        rapidjson::Value::MemberIterator externalURLIt = document.FindMember("external-url");
        if (externalURLIt != document.MemberEnd() && externalURLIt->value.IsString())
            externalURL = std::string(externalURLIt->value.GetString(), externalURLIt->value.GetStringLength());
        else
        {
            externalURL = "127.0.0.1";
            LOG_WARNING("Missing 'external-url'. External Url will be set to default '{0}'.", externalURL);
        }

        // Load thread counts
        rapidjson::Value::MemberIterator threadCountIt = document.FindMember("thread-count");
        if (threadCountIt != document.MemberEnd() && threadCountIt->value.IsUint())
            threadCount = std::max(threadCountIt->value.GetUint(), 1u);
        else
        {
            threadCount = boost::thread::hardware_concurrency() - 1;
            LOG_WARNING("Missing 'thread-count'. Thread count will be set to default '{0}'.", threadCount);
        }

        if (threadCount == 1)
        {
            LOG_WARNING("No additional worker thread is used. This could cause some slowdowns.");
        }

        return true;
    }
}