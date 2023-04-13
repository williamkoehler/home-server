#pragma once
#include "libraries/boost.hpp"

namespace server
{
    namespace config
    {
#define APP_NAME "home."
#define CONFIGURATION_FILE_NAME "core-info.json"

        /// @brief Root directory
        ///
        /// @return Directory path
        inline boost::filesystem::path GetRootDirectory()
        {
            // return boost::filesystem::path("/etc/home-automation");
            return boost::filesystem::path("./");
        }

        /// @brief Directory in which configuration files are stored
        ///
        /// @return Directory path
        inline boost::filesystem::path GetConfigurationDirectory()
        {
            return GetRootDirectory() / "config";
        }

        /// @brief Directory in which state specific files are stored
        ///
        /// @return Directory path
        inline boost::filesystem::path GetDataDirectory()
        {
            return GetRootDirectory() / "data";
        }

        /// @brief Directory in which native scripts are stored
        ///
        /// @return Directory path
        inline boost::filesystem::path GetScriptDirectory()
        {
            return GetDataDirectory() / "scripts";
        }

        /// @brief Directory in which log files are stored
        ///
        /// @return Directory path
        inline boost::filesystem::path GetLogDirectory()
        {
            return GetRootDirectory() / "log";
        }
    }
}