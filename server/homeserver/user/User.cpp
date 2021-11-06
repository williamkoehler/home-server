#include "User.hpp"
#include "UserManager.hpp"
#include "../Core.hpp"
#include "../tools.hpp"
#include "../database/Database.hpp"

namespace server
{
	std::string StringifyUserAccessLevel(UserAccessLevel accessLevel)
	{
		switch (accessLevel)
		{
		case UserAccessLevel::kNormalUserAccessLevel:
			return "normal";
		case UserAccessLevel::kMaintainerUserAccessLevel:
			return "maintainer";
		case UserAccessLevel::kAdministratorUserAccessLevel:
			return "admin";
		default:
			return "restricted";
		}
	}
	UserAccessLevel ParseUserAccessLevel(const std::string& accessLevel)
	{
		if (accessLevel == "normal")
			return UserAccessLevel::kNormalUserAccessLevel;
		else if (accessLevel == "maintainer")
			return UserAccessLevel::kMaintainerUserAccessLevel;
		else if (accessLevel == "admin")
			return UserAccessLevel::kAdministratorUserAccessLevel;
		else
			return UserAccessLevel::kRestrictedUserAccessLevel;
	}

	User::User(const std::string& name, identifier_t userID, uint8_t h[SHA256_DIGEST_LENGTH], uint8_t s[SALT_LENGTH], UserAccessLevel& accessLevel)
		: name(name), userID(userID), accessLevel(accessLevel)
	{
		memcpy(hash, h, SHA256_DIGEST_LENGTH);
		memcpy(salt, s, SALT_LENGTH);
	}
	User::~User()
	{
	}

	std::string User::GetName()
	{
		boost::shared_lock_guard lock(mutex);
		return name;
	}
	bool User::SetName(const std::string& v)
	{
		boost::lock_guard lock(mutex);
		if (Database::GetInstance()->UpdateUserPropName(shared_from_this(), name, v))
		{
			name = v;
			return true;
		}
		return false;
	}

	void User::GetHash(uint8_t* h)
	{
		assert(h != nullptr);
		boost::shared_lock_guard lock(mutex);
		memcpy(h, hash, SHA256_DIGEST_LENGTH);
	}
	bool User::CompaireHash(uint8_t h[SHA256_DIGEST_LENGTH])
	{
		boost::shared_lock_guard lock(mutex);
		return memcmp(hash, h, SHA256_DIGEST_LENGTH) == 0;
	}
	bool User::SetHash(uint8_t h[SHA256_DIGEST_LENGTH])
	{
		boost::lock_guard lock(mutex);
		if (Database::GetInstance()->UpdateUserPropHash(shared_from_this(), hash, h))
		{
			memcpy(hash, h, SHA256_DIGEST_LENGTH);
			return true;
		}
		return false;
	}

	void User::GetSalt(uint8_t* s)
	{
		assert(s != nullptr);
		boost::shared_lock_guard lock(mutex);
		memcpy(s, salt, SALT_LENGTH);
	}

	UserAccessLevel User::GetAccessLevel()
	{
		boost::shared_lock_guard lock(mutex);
		return accessLevel;
	}
	bool User::SetAccessLevel(UserAccessLevel v)
	{
		boost::lock_guard lock(mutex);
		if (Database::GetInstance()->UpdateUserPropAccessLevel(shared_from_this(), accessLevel, v))
		{
			accessLevel = v;
			return true;
		}
		return false;
	}
}
