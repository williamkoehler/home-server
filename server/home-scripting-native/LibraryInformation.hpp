#pragma once
#include "common.hpp"
#include <home-scripting/ScriptSource.hpp>
#include <home-scripting/View.hpp>

namespace server
{
    namespace scripting
    {
        namespace native
        {
            class NativeScriptImpl;

            template <typename T = NativeScriptImpl>
            using CreateScriptCallback = Ref<T> (*)();

            template <class T>
            union CreateScriptCallbackConversion
            {
                CreateScriptCallback<T> f1;
                CreateScriptCallback<> f2;
            };

            /// @brief Script information necessary to dynamically create script instance
            ///
            struct ScriptInformation
            {
                std::string scriptName; // Secondary name that links the script with corresponding database entry!
                                        // THIS SHOULD NOT BE CHANGED AFTER LIBRARY RELEASE!
                std::string name;
                ScriptUsage usage;

                CreateScriptCallback<> callback;

                template <typename T>
                inline static ScriptInformation Build(const std::string& scriptName, const std::string& name,
                                                      ScriptUsage usage, CreateScriptCallback<T> callback)
                {
                    return ScriptInformation{
                        .scriptName = scriptName,
                        .name = name,
                        .usage = usage,
                        .callback = (CreateScriptCallbackConversion<T>{callback}).f2,
                    };
                }

                template <typename T>
                inline static ScriptInformation Build(const std::string& scriptName, const std::string& name,
                                                      ScriptUsage usage)
                {
                    return ScriptInformation{
                        .scriptName = scriptName,
                        .name = name,
                        .usage = usage,
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
                    // Build strinng
                    std::stringstream ss;
                    ss << major << '.' << minor << '.' << patch << '-' << revision;

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