#include "common.hpp"
#include "plugin/PluginManager.hpp"
#include "Core.hpp"
#include <openssl/ssl.h>
#include <openssl/opensslv.h>
#include "Version.hpp"

#include <libquickmail/quickmail.h>

#include <boost/process.hpp>

using namespace server;

int main(int argc, char** argv)
{
	try
	{
		{
			char line[14] = "             ";

			memcpy(line, SERVER_VERSION, strlen(SERVER_VERSION));

			LOG_INFO("\n"
				"\t ____________________________________\n"
				"\t|                                    |\n"
				"\t|       Home Automation Server       |\n"
				"\t|       Version {0}        |\n"
				"\t|                                    |\n"
				"\t|       Made by :                    |\n"
				"\t|       - William KOEHLER            |\n"
				"\t|                                    |\n"
				"\t|       This server is using :       |\n"
				"\t|       - spdlog                     |\n"
				"\t|       - Boost                      |\n"
				"\t|       - openssl                    |\n"
				"\t|       - rapidjson                  |\n"
				"\t|       - jwt-cpp                    |\n"
				"\t|       - xxHash                     |\n"
				"\t|       - cppcodec                   |\n"
				"\t|       - duktape                    |\n"
				"\t|       - libquickmail               |\n"
				"\t|____________________________________|\n", line);
		}

#ifdef _DEBUG
		LOG_WARNING("Running in debug mode");
#endif

#ifdef _WIN32
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData))
		{
			LOG_ERROR("Initialize WinSock 2.2");
			throw std::runtime_error("Initialize WinSock");
		}
		LOG_INFO("Using WinSock 2.2");
#endif
		SSL_load_error_strings();
		OpenSSL_add_all_algorithms();
		LOG_INFO("Using {0}", OPENSSL_VERSION_TEXT);

		LOG_INFO("Using Boost v{0}", BOOST_LIB_VERSION);

		if (quickmail_initialize())
			LOG_WARNING("Initialize LibQuickMail");
		LOG_INFO("Using LibQuickMail v{0}", quickmail_get_version());

		LOG_INFO("Starting HomeAutomation server v{0}", SERVER_VERSION);


		//Init plugin
		Ref<PluginManager> pluginManager = PluginManager::Create();
		if (pluginManager == nullptr)
			throw std::runtime_error("Fatal error");

		Ref<Core> core = Core::Create();
		if (core == nullptr)
			throw std::runtime_error("Fatal error");

		core->Run();

		LOG_INFO("Stopping HomeAutomation server");

		// Make sure that core is released before pluginManager
		core = nullptr;
		pluginManager = nullptr;

#ifdef _WIN32
		WSACleanup();
#endif
	}
	catch (std::exception e)
	{
#ifdef _DEBUG
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