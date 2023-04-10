#include "NativeScriptProvider.hpp"
#include "LibraryInformation.hpp"
#include "NativeScript.hpp"
#include "NativeScriptSource.hpp"
#include <home-scripting/ScriptManager.hpp>

namespace server
{
    namespace scripting
    {
        namespace native
        {
            NativeScriptProvider::NativeScriptProvider(const std::string& path) : path(path)
            {
            }
            NativeScriptProvider::~NativeScriptProvider()
            {
            }
            Ref<ScriptProvider> NativeScriptProvider::Create(const std::string& path)
            {
                Ref<NativeScriptProvider> provider = boost::make_shared<NativeScriptProvider>(path);

                const char* libraryFunction = "GetLibraryInformations";

                LOG_INFO("Initializing native script provider in '{0}'.", path);

                try
                {
                    // Iterate over every library
                    for (boost::filesystem::recursive_directory_iterator it(path);
                         it != boost::filesystem::recursive_directory_iterator(); it++)
                    {
                        std::string libraryFileName = it->path().stem().string();

                        Ref<boost::dll::shared_library> library = boost::make_shared<boost::dll::shared_library>();

                        boost::system::error_code ec;
                        library->load(it->path(), boost::dll::load_mode::default_mode, ec);

                        if (!ec)
                        {
                            if (library->has(libraryFunction))
                            {
                                try
                                {
                                    LOG_INFO("Registering library '{0}'", libraryFileName);

                                    //  Get lib informations
                                    LibraryInformation lib =
                                        library->get<GetLibraryInformationsCallback>(libraryFunction)();

                                    // Log library details
                                    {
                                        std::stringstream ss;

                                        // Library Name: foo-foo--doo-doo
                                        // Name: FooFoo DooDoo
                                        // License: MIT
                                        // Version: 1.0.0.0
                                        // Authors: Max Mustermann, ...
                                        // Dependencies: boost, ...

                                        ss << "Library name: " << lib.libraryName << std::endl;
                                        ss << "Name:         " << lib.name << std::endl;
                                        ss << "License:      " << lib.license << std::endl;
                                        ss << "Version:      " << lib.version.ToString() << std::endl;
                                        ss << "Authors:      " << boost::join(lib.authors, ", ") << std::endl;
                                        ss << "Dependencies: " << boost::join(lib.dependencies, ", ") << std::endl;

                                        LOG_INFO("Library {0}\n{1}", libraryFileName, ss.str());
                                    }

                                    // Add library
                                    if (!lib.libraryName.empty())
                                    {
                                        provider->libraryList.push_back(library);

                                        // Add scripts
                                        for (ScriptInformation& scriptInformation : lib.scripts)
                                        {
                                            // Generate unique name for each script
                                            std::string name = lib.libraryName + '-' + scriptInformation.scriptName;

                                            // Add static script
                                            provider->scriptList[name] = scriptInformation;

                                            // Log script details
                                            {
                                                std::stringstream ss;

                                                // Script Name: foo-foo--boo-boo
                                                // Name: FooFoo BooBoo
                                                // Usage: No usage...

                                                ss << "Script name: " << scriptInformation.scriptName << std::endl;
                                                ss << "Name:        " << scriptInformation.name << std::endl;

                                                LOG_INFO("Static Script {0}\n{1}", scriptInformation.scriptName,
                                                         ss.str());
                                            }
                                        }
                                    }
                                    else
                                    {
                                        LOG_ERROR("Invalid library name. Library '{0}'", libraryFileName);
                                    }
                                }
                                catch (std::exception)
                                {
                                    LOG_ERROR("'GetLibraryInformations' not working properly. Library '{0}'",
                                              libraryFileName);
                                }
                            }
                            else
                            {
                                LOG_ERROR("'GetLibraryInformations' is missing. Library '{0}'", libraryFileName);
                            }
                        }
                        else
                        {
                            LOG_ERROR("Load or open Library '{0}'", libraryFileName);
                        }
                    }
                }
                catch (std::exception e)
                {
                    LOG_ERROR("Iterate over libraries in '{0}'\n{1}", path, e.what());
                }

                return provider;
            }

            boost::container::vector<StaticScriptSource> NativeScriptProvider::GetStaticScriptSources()
            {
                boost::container::vector<StaticScriptSource> scriptSourceList =
                    boost::container::vector<StaticScriptSource>();

                // Generate static script sources
                for (auto& [name, scriptInformation] : scriptList)
                {
                    scriptSourceList.push_back(StaticScriptSource{name, scriptInformation.flags});
                }

                return scriptSourceList;
            }

            Ref<ScriptSource> NativeScriptProvider::CreateScriptSource(identifier_t id, const std::string& name,
                                                                       const std::string_view& content)
            {
                (void)content;

                robin_hood::unordered_node_map<std::string, ScriptInformation>::const_iterator it =
                    scriptList.find(name);

                if (it != scriptList.end())
                {
                    ScriptInformation scriptInformation = it->second;

                    // Remove created scripts
                    scriptList.erase(it);

                    return NativeScriptSource::Create(id, scriptInformation.name, scriptInformation.flags,
                                                      scriptInformation.callback);
                }
                else
                    return nullptr;
            }
        }
    }
}