#pragma once
#include "common.hpp"
#include <sstream>

namespace server
{
    namespace scripting
    {
        namespace sdk
        {
            class Script;

            template <typename T = Script>
            using CreateScriptCallback = Ref<T> (*)();

            template <class T>
            union CreateScriptCallbackConversion
            {
                CreateScriptCallback<T> f1;
                CreateScriptCallback<> f2;
            };

            enum ScriptFlags
            {
                kScriptFlag_None = 0x00,
                kScriptFlag_RoomSupport = 0x01,
                kScriptFlag_DeviceSupport = 0x02,
                kScriptFlag_ServiceSupport = 0x04,
            };

            /// @brief Script information necessary to dynamically create script instance
            ///
            struct ScriptInformation
            {
                std::string scriptName; // Secondary name that links the script with corresponding database entry!
                                        // THIS SHOULD NOT BE CHANGED AFTER LIBRARY RELEASE!
                std::string name;

                /// @brief Script flags
                /// @see server::scripting::sdk::ScriptFlags
                ///
                uint8_t flags;

                /// @brief Script implementation create callback
                ///
                CreateScriptCallback<> callback;

                template <typename T>
                inline static ScriptInformation Build(const std::string& scriptName, const std::string& name,
                                                      uint8_t flags, CreateScriptCallback<T> callback)
                {
                    return ScriptInformation{
                        .scriptName = scriptName,
                        .name = name,
                        .flags = flags,
                        .callback = (CreateScriptCallbackConversion<T>{callback}).f2,
                    };
                }

                template <typename T>
                inline static ScriptInformation Build(const std::string& scriptName, const std::string& name,
                                                      uint8_t flags)
                {
                    return ScriptInformation{
                        .scriptName = scriptName,
                        .name = name,
                        .flags = flags,
                        .callback = (CreateScriptCallbackConversion<T>{&T::Create}).f2,
                    };
                }
            };

            /// @brief Script library version (major.minor.patch.revision)
            ///
            class LibraryVersion
            {
              public:
                uint32_t major;
                uint32_t minor;
                uint32_t patch;
                uint32_t revision;

                /// @brief Convert library version to string
                ///
                /// @return Library version
                inline std::string ToString() const
                {
                    // Build string
                    std::stringstream ss;

                    if (revision == 0)
                        ss << major << '.' << minor << '.' << patch;
                    else
                        ss << major << '.' << minor << '.' << patch << '-pre' << revision;

                    return ss.str();
                }
            };

            /// @brief Library information
            ///
            struct LibraryInformation
            {
                /// @brief Secondary name that links the script with corresponding database entry! THIS
                ///        SHOULD NOT BE CHANGED AFTER LIBRARY RELEASE!
                ///
                std::string libraryName;

                std::string name;
                LibraryVersion version;
                std::string license;
                boost::container::vector<std::string> authors;
                boost::container::vector<std::string> dependencies;

                boost::container::vector<ScriptInformation> scripts;
            };

            using GetLibraryInformationsCallback = LibraryInformation();
        }
    }
}