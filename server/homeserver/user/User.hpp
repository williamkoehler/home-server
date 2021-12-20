#pragma once
#include "../common.hpp"

namespace server
{
	class UserManager;

	class Database;

	class JsonApi;

	enum class UserAccessLevel
	{
		kRestrictedUserAccessLevel,
		kNormalUserAccessLevel,
		kMaintainerUserAccessLevel,
		kAdministratorUserAccessLevel,
	};

	std::string StringifyUserAccessLevel(UserAccessLevel accessLevel);
	UserAccessLevel ParseUserAccessLevel(const std::string& accessLevel);

	class User : public boost::enable_shared_from_this<User>
	{
	private:
		friend class UserManager;
		friend class Database;
		friend class JsonApi;

		boost::shared_mutex mutex;

		std::string name;
		const identifier_t userID;
		uint8_t hash[SHA256_DIGEST_LENGTH] = "";
		uint8_t salt[SALT_LENGTH] = "";

		UserAccessLevel accessLevel;

		// Settings

	public:
		User(const std::string& name, identifier_t id, uint8_t hash[SHA256_DIGEST_LENGTH], uint8_t salt[SALT_LENGTH], UserAccessLevel accessLevel);
		~User();
		static Ref<User> Create(const std::string& name, identifier_t id, uint8_t hash[SHA256_DIGEST_LENGTH], uint8_t salt[SALT_LENGTH], UserAccessLevel accessLevel);

		std::string GetName();
		bool SetName(const std::string& v);

		inline uint32_t GetUserID() const { return userID; }

		void GetHash(uint8_t* h);
		bool CompaireHash(uint8_t h[SHA256_DIGEST_LENGTH]);
		bool SetHash(uint8_t h[SHA256_DIGEST_LENGTH]);

		void GetSalt(uint8_t* s);

		UserAccessLevel GetAccessLevel();
		bool SetAccessLevel(UserAccessLevel v);
	};
}

