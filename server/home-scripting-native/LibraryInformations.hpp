#pragma once
#include "common.hpp"

namespace server
{
    namespace scripting
    {
        namespace native
        {
            struct LibraryVersion
            {
                int major;
                int minor;
                int patch;
                int revision;
            };

            struct LibraryInformations
            {
                std::string libraryName;
                std::string name;
                LibraryVersion version;
                std::string license;
                boost::container::vector<std::string> authors;
                boost::container::vector<std::string> dependencies;
            };

            using GetLibraryInformationsCallback = LibraryInformations();
        }
    }
}