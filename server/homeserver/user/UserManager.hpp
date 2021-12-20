#pragma once
#include "../common.hpp"
#include <jwt-cpp/jwt.h>

namespace server
{
	class Core;
	enum class UserAccessLevel;
	class User;

	class JsonApi;
	class SSHSession;

	class UserManager : public boost::enable_shared_from_this<UserManager>
	{
	private:
		friend class JsonApi;
		friend class SSHSession;

		boost::shared_mutex mutex;

		boost::atomic<time_t> timestamp = 0;

		boost::unordered::unordered_map<identifier_t, Ref<User>> userList;

		//Authentication
		uint8_t* authenticationKey = nullptr;
		jwt::verifier<jwt::default_clock> verifier;

		void GenerateHash(std::string_view passwd, uint8_t* salt, uint8_t* digest);

		// Database
		bool LoadUser(identifier_t userID, const std::string& name, uint8_t hash[SHA256_DIGEST_LENGTH], uint8_t salt[SALT_LENGTH], UserAccessLevel accessLevel);

	public:
		UserManager();
		~UserManager();
		static Ref<UserManager> Create();
		static Ref<UserManager> GetInstance();

		//Timestamp
		void UpdateTimestamp();
		time_t GetLastTimestamp() const
		{
			return timestamp;
		}

		//User
		Ref<User> AddUser(std::string name, std::string passwd, UserAccessLevel accessLevel);
		Ref<User> AddUser(std::string name, uint8_t* hash, uint8_t* salt, UserAccessLevel accessLevel);

		size_t GetUserCount()
		{
			boost::shared_lock_guard lock(mutex);
			return userList.size();
		}
		Ref<User> GetUser(identifier_t userID);
		Ref<User> GetUserByName(std::string_view name);
		Ref<User> GetUserByPassword(std::string_view name, std::string_view password);
		bool SetUserPassword(identifier_t userID, std::string_view passwd, std::string_view newPasswd);
		bool SetUserPassword(const Ref<User>& user, std::string_view passwd, std::string_view newPasswd);

		bool RemoveUser(identifier_t userID);

		//JWT
		std::string GenerateToken(std::string_view name, std::string_view passwd);
		identifier_t VerifyToken(jwt::decoded_jwt& decoded);
	};
}