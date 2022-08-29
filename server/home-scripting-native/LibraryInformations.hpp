#pragma once
#include "common.hpp"

namespace server
{
    namespace scripting
    {
        namespace native
        {
            /// @brief Script library version (major.minor.patch.revision)
            /// 
            struct LibraryVersion
            {
                uint32_t major;
                uint32_t minor;
                uint32_t patch;
                uint32_t revision;
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

            using GetLibraryInformationsCallback = void(LibraryInformations* lib);
        }
    }
}