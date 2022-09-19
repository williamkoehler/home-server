#pragma once
#include "common.hpp"
#include <home-database/Database.hpp>
#include <home-main/Home.hpp>
#include <home-networking/NetworkManager.hpp>
#include <home-scripting/ScriptManager.hpp>
#include <home-common/Worker.hpp>
#include <home-users/UserManager.hpp>

namespace server
{
    /// @brief The core of the server, that manages every part of the server
    ///
    class Core : public boost::enable_shared_from_this<Core>
    {
      private:
        std::string name = "error-no-name";

        // Networking
        std::string externalURL;
        std::string address;
        uint16_t port;

        // Scripting
        std::string nativeScriptDirectory;

        // Components
        Ref<Worker> worker;
        Ref<Database> database;
        Ref<scripting::ScriptManager> scriptManager;
        Ref<main::Home> home;
        Ref<users::UserManager> userManager;
        Ref<networking::NetworkManager> networkManager;

        /// @brief Load configurations from file
        ///
        /// @return Successfulness
        bool Load();

      public:
        Core();
        virtual ~Core();
        static Ref<Core> Create();
        static Ref<Core> GetInstance();

        inline bool IsRunning() const
        {
            return worker->IsRunning();
        }

        inline const std::string& GetName()
        {
            return name;
        }

        // Networking
        inline const std::string& GetAddress()
        {
            return address;
        }
        inline uint16_t GetPort()
        {
            return port;
        }
        inline const std::string& GetExternalURL()
        {
            return externalURL;
        }

        void Run();

        // Shutdown
        void Shutdown();
    };

    std::string GetServerName();
}
