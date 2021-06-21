#pragma once
#include "../common.h"

namespace server
{
	class JsonApi;

	class EMail : public boost::enable_shared_from_this<EMail>
	{
	private:
		friend class JsonApi;

		boost::mutex mutex;

		std::string server;
		uint16_t port;
		std::string username, password;
		boost::container::vector<std::string> recipients;

	public:
		EMail(std::string server, uint16_t port, std::string username, std::string password);
		~EMail();
		static Ref<EMail> Create(std::string server, uint16_t port, std::string username, std::string password);
		static Ref<EMail> GetInstance();

		void AddRecipient(std::string recipient);

		void Send(std::string title, std::string body);
	};
}