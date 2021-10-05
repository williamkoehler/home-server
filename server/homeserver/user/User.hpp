#pragma once
#include "../common.hpp"

#define SALT_LENGTH 16

namespace server
{
	class UserManager;

	class JsonApi;
	class SSHSession;

	enum class UserAccessLevels
	{
		kRestrictedUserAccessLevel,
		kNormalUserAccessLevel,
		kAdministratorUserAccessLevel,
	};

	class User : public boost::enable_shared_from_this<User>
	{
	private:
		friend class UserManager;
		friend class JsonApi;
		friend class SSHSession;

		boost::shared_mutex mutex;

		std::string name;
		const uint32_t userID;
		uint8_t hash[SHA256_DIGEST_LENGTH] = "";
		uint8_t salt[SALT_LENGTH] = "";

		UserAccessLevels accessLevel;

		// Settings

	public:
		User(std::string& name, uint32_t& id, uint8_t hash[SHA256_DIGEST_LENGTH], uint8_t salt[SALT_LENGTH], UserAccessLevels& accessLevel);
		~User();
		static Ref<User> Create(std::string name, uint32_t userID, uint8_t hash[SHA256_DIGEST_LENGTH], uint8_t salt[SALT_LENGTH], UserAccessLevels accessLevel);

		inline std::string GetName()
		{
			boost::shared_lock_guard lock(mutex);
			return name;
		}
		inline void SetName(std::string v)
		{
			boost::lock_guard lock(mutex);
			name = v;
		}

		inline uint32_t GetUserID() const { return userID; }

		inline void GetHash(uint8_t* h)
		{
			assert(h != nullptr);
			boost::shared_lock_guard lock(mutex);
			memcpy(h, hash, SHA256_DIGEST_LENGTH);
		}
		inline bool CompaireHash(uint8_t h[SHA256_DIGEST_LENGTH])
		{
			boost::shared_lock_guard lock(mutex);
			return memcmp(hash, h, SHA256_DIGEST_LENGTH) == 0;
		}
		inline void SetHash(uint8_t h[SHA256_DIGEST_LENGTH])
		{
			boost::lock_guard lock(mutex);
			memcpy(hash, h, SHA256_DIGEST_LENGTH);
		}

		inline void GetSalt(uint8_t* s)
		{
			assert(s != nullptr);
			boost::shared_lock_guard lock(mutex);
			memcpy(s, salt, SALT_LENGTH);
		}

		inline UserAccessLevels GetAccessLevel()
		{
			boost::shared_lock_guard lock(mutex);
			return accessLevel;
		}
		inline void SetAccessLevel(UserAccessLevels v)
		{
			boost::lock_guard lock(mutex);
			accessLevel = v;
		}
	};
}

