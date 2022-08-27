#include "NativeScriptProvider.hpp"
#include "LibraryInformations.hpp"
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

                LOG_INFO("Initializing native script provider in '{0}'.", path);

                if (provider != nullptr)
                {
                    // Load libraries
                    provider->LoadLibraries();
                }

                return provider;
            }

            Ref<ScriptSource> NativeScriptProvider::CreateScriptSource(identifier_t id, const std::string& name,
                                                                       ScriptUsage usage, const std::string_view& data)
            {
                // Lock main mutex
                boost::lock_guard lock(mutex);

                std::string::size_type seperator = name.find_first_of('/');

                if (seperator != std::string::npos && (seperator + 1) <= name.size())
                {
                    std::string libraryName = name.substr(0, seperator);
                    std::string scriptName = name.substr(seperator + 1);

                    // Find library
                    const robin_hood::unordered_node_map<std::string, Ref<boost::dll::shared_library>>::const_iterator
                        it = libraryList.find(libraryName);
                    if (it != libraryList.end())
                    {
                        Ref<boost::dll::shared_library> library = it->second;

                        // Find create script callback
                        std::string callbackName = "Create" + scriptName;
                        if (library->has(callbackName.c_str()))
                        {
                            CreateScriptCallback* callback = library->get<CreateScriptCallback>(callbackName);

                            // Create script source
                            return NativeScriptSource::Create(id, scriptName, usage, callback);
                        }
                    }
                }

                return nullptr;
            }

            void NativeScriptProvider::LoadLibraries()
            {
                try
                {
                    // Iterate over every library
                    for (boost::filesystem::recursive_directory_iterator it(path);
                         it != boost::filesystem::recursive_directory_iterator(); it++)
                    {
                        std::string name = it->path().stem().string();

                        Ref<boost::dll::shared_library> library = boost::make_shared<boost::dll::shared_library>();

                        boost::system::error_code ec;
                        library->load(it->path(), boost::dll::load_mode::default_mode, ec);

                        if (!ec)
                        {
                            if (library->has("GetLibraryInformations"))
                            {
                                try
                                {
                                    LOG_INFO("Registering library '{0}'", name);

                                    LibraryInformations informations =
                                        library->get<GetLibraryInformationsCallback>("GetLibraryInformations")();

                                    std::stringstream ss;

                                    // Name: FooFoo DooDoo
                                    // License: MIT
                                    // Version: 1.0.0.0
                                    // Authors: Max Mustermann, ...
                                    // Dependencies: boost, ...

                                    ss << "Library name: " << informations.libraryName << std::endl;
                                    ss << "Name:         " << informations.name << std::endl;
                                    ss << "License:      " << informations.license << std::endl;
                                    ss << "Version:      " << informations.version.major << "."
                                       << informations.version.minor << "." << informations.version.patch << "."
                                       << informations.version.revision << std::endl;
                                    ss << "Authors:      " << boost::join(informations.authors, ", ") << std::endl;
                                    ss << "Dependencies: " << boost::join(informations.dependencies, ", ") << std::endl;

                                    LOG_INFO("Library {0}\n{1}", name, ss.str());

                                    if (!informations.libraryName.empty())
                                        libraryList[informations.libraryName] = library;
                                    else
                                    {
                                        LOG_ERROR("Invalid library name. Library '{0}'", name);
                                    }
                                }
                                catch (std::exception)
                                {
                                    LOG_ERROR("'GetLibraryInformations' not working properly. Library '{0}'", name);
                                }
                            }
                            else
                            {
                                LOG_ERROR("'GetLibraryInformations' is missing. Library '{0}'", name);
                            }
                        }
                        else
                        {
                            LOG_ERROR("Load or open Library '{0}'", name);
                        }
                    }
                }
                catch (std::exception e)
                {
                    LOG_ERROR("Iterate over libraries in '{0}'\n{1}", path, e.what());
                }
            }
        }
    }
}