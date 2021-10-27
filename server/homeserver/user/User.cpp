#include "User.hpp"
#include "UserManager.hpp"
#include "../Core.hpp"
#include "../tools.hpp"

namespace server
{
	User::User(const std::string& name, identifier_t userID, uint8_t h[SHA256_DIGEST_LENGTH], uint8_t s[SALT_LENGTH], UserAccessLevels& accessLevel)
		: name(name), userID(userID), accessLevel(accessLevel)
	{
		memcpy(hash, h, SHA256_DIGEST_LENGTH);
		memcpy(salt, s, SALT_LENGTH);
	}
	User::~User()
	{
	}
	Ref<User> User::Create(const std::string& name, identifier_t userID, uint8_t hash[SHA256_DIGEST_LENGTH], uint8_t salt[SALT_LENGTH], UserAccessLevels accessLevel)
	{
		return boost::make_shared<User>(name, userID, hash, salt, accessLevel);
	}
}
