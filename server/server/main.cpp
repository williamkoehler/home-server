#include "core.hpp"
#include <common/common.hpp>

#include <openssl/ssl.h>

#include <boost/process.hpp>

using namespace server;

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    // Creating necessary folders
    try
    {
        boost::filesystem::create_directories(config::GetRootDirectory());
        boost::filesystem::create_directories(config::GetDataDirectory());
        boost::filesystem::create_directories(config::GetScriptDirectory());
        boost::filesystem::create_directories(config::GetLogDirectory());
    }
    catch (std::exception e)
    {
        printf("Failed to create directories. Please check file permissions.\n");
        return -1;
    }

    try
    {
        // Create logger
        Log::Create();
    }
    catch (std::exception e)
    {
        printf("Failed to initialize logger. Please check file permissions.\n");
        return -1;
    }

    try
    {
        // Log hello message
        {
            char line[14];
            memset(line, ' ', sizeof(line) - 1);
            memcpy(line, SERVER_VERSION, strlen(SERVER_VERSION));

            LOG_INFO("\n"
                     "\t __________________________________________\n"
                     "\t|                                          |\n"
                     "\t|                                          |\n"
                     "\t|          Home Automation Server          |\n"
                     "\t|          Version {0}           |\n"
                     "\t|                                          |\n"
                     "\t|          Made by :                       |\n"
                     "\t|          - William KOEHLER               |\n"
                     "\t|                                          |\n"
                     "\t|          This server is using :          |\n"
                     "\t|          - spdlog                        |\n"
                     "\t|          - Boost                         |\n"
                     "\t|          - openssl                       |\n"
                     "\t|          - rapidjson                     |\n"
                     "\t|          - jwt-cpp                       |\n"
                     "\t|          - xxHash                        |\n"
                     "\t|          - cppcodec                      |\n"
                     "\t|          - duktape                       |\n"
                     "\t|                                          |\n"
                     "\t|__________________________________________|\n",
                     line);
        }

#ifndef NDEBUG
        LOG_WARNING("Running in debug mode");
#endif

        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        LOG_INFO("Using {0}", OPENSSL_VERSION_TEXT);

        LOG_INFO("Using Boost v{0}", BOOST_LIB_VERSION);

        LOG_INFO("Starting HomeAutomation server v{0}", SERVER_VERSION);

        Ref<Core> core = Core::Create();
        if (core == nullptr)
            throw std::runtime_error("Fatal error");

        core->Run();

        LOG_INFO("Stopping HomeAutomation server");

        // Make sure that core is released before pluginManager
        core = nullptr;
    }
    catch (std::exception e)
    {
        LOG_ERROR("Ops... something terrible must have happened...\n");

#ifndef NDEBUG
        printf("Press a key to close. For more details press D.\n");
        int key = std::cin.get();
        if (key == 'd' || key == 'D')
        {
            printf("Exception : %s\n", e.what());
            std::cin.get();
        }
#endif
        return -1;
    }

    return 0;
}