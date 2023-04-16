#pragma once
#include "common.hpp"
#include <home-api/NetworkManager.hpp>
#include <home-api/UserManager.hpp>
#include <home-common/Worker.hpp>
#include <home-database/Database.hpp>
#include <home-main/Home.hpp>
#include <home-scripting/ScriptManager.hpp>

namespace server
{
    struct CoreConfig
    {
        std::string name = "error-no-name";

        struct DatabaseConfig
        {
            DatabaseType type = DatabaseType::kSQLiteDatabaseType;
            std::string location;
            std::string username;
            std::string password;
        } database;

        struct NetworkingConfig
        {
            std::string externalURL;
            std::string address;
            uint16_t port;
        } networking;

        struct ScriptingConfig
        {
            struct
            {
                std::string source;
            } nativeScript;
        } scripting;
    };

    /// @brief The core of the server, that manages every part of the server
    ///
    class Core : public boost::enable_shared_from_this<Core>
    {
      private:
        std::string name = "error-no-name";

        // Components
        Ref<Worker> worker;
        Ref<Database> database;
        Ref<scripting::ScriptManager> scriptManager;
        Ref<main::Home> home;
        Ref<api::UserManager> userManager;
        Ref<api::NetworkManager> networkManager;

        /// @brief Load configurations from file
        ///
        /// @return Successfulness
        bool Load(CoreConfig& config);

      public:
        Core();
        virtual ~Core();

        /// @brief Create core instance
        ///
        /// @return Core singleton
        static Ref<Core> Create();

        /// @brief Get core instance
        ///
        /// @return Core singleton
        static Ref<Core> GetInstance();

        /// @brief Get server status
        ///
        /// @return true Server is running
        /// @return false Server is not running
        inline bool IsRunning() const
        {
            return worker->IsRunning();
        }

        /// @brief Get server name
        ///
        /// @return Server name
        inline const std::string& GetName()
        {
            return name;
        }

        /// @brief Run server
        ///
        void Run();

        /// @brief Shutdown server
        ///
        void Shutdown();
    };

    std::string GetServerName();
}
