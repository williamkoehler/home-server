#include "EMail.h"
#include <libquickmail/quickmail.h>

namespace server
{
	boost::weak_ptr<EMail> instanceEMail;

	EMail::EMail(std::string server, uint16_t port, std::string username, std::string password)
		: server(std::move(server)), port(port),
		username(std::move(username)), password(std::move(password))
	{
	}
	EMail::~EMail()
	{
	}
	Ref<EMail> EMail::Create(std::string server, uint16_t port, std::string username, std::string password)
	{
		if (!instanceEMail.expired())
			return Ref<EMail>(instanceEMail);

		Ref<EMail> email = boost::make_shared<EMail>(std::move(server), port, std::move(username), std::move(password));
		instanceEMail = email;

		return email;
	}
	
	Ref<EMail> EMail::GetInstance()
	{
		return Ref<EMail>(instanceEMail);
	}
	
	void EMail::AddRecipient(std::string recipient)
	{
		boost::lock_guard lock(mutex);

		EMail::recipients.push_back(recipient);
	}
	void EMail::Send(std::string title, std::string body)
	{
		boost::lock_guard lock(mutex);

		const quickmail mail = quickmail_create(username.c_str(), title.c_str());
		if (mail == nullptr)
		{
			LOG_ERROR("Send email");
			return;
		}

		for (std::string& recipient : recipients)
			quickmail_add_to(mail, recipient.c_str());
		quickmail_set_body(mail, body.c_str());

		const char* result = quickmail_send_secure(mail, server.c_str(), port, username.c_str(), password.c_str());
		if (result != nullptr)
			LOG_ERROR("Send email : {0}", result);

		quickmail_destroy(mail);
	}
}