#pragma once
#include "boost.hpp"

namespace server
{
    namespace config
    {
#define APP_NAME "home."
#define CONFIGURATION_FILE_NAME "core-info.json"

        /// @brief Directory in which configuration files are stored
        ///
        /// @return Directory path
        inline boost::filesystem::path GetConfigurationDirectory()
        {
            return boost::filesystem::path("./config");
        }

        /// @brief Directory in which state specific files are stored
        ///
        /// @return Directory path
        inline boost::filesystem::path GetStateDirectory()
        {
            return boost::filesystem::path("./data");
        }

        /// @brief Directory in which native scripts are stored
        ///
        /// @return Directory path
        inline boost::filesystem::path GetScriptDirectory()
        {
            return GetStateDirectory() / "scripts";
        }

        /// @brief Directory in which log files are stored
        ///
        /// @return Directory path
        inline boost::filesystem::path GetLogDirectory()
        {
            return boost::filesystem::path("./log");
        }
    }
}