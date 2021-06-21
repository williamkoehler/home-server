#pragma once
#include "../common.h"
#include <jwt-cpp/jwt.h>

namespace server
{
	class Core;
	enum class UserAccessLevels;
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

		boost::unordered::unordered_map<uint32_t, Ref<User>> userList;

		//Authentication
		uint8_t* authenticationKey = nullptr;
		jwt::verifier<jwt::default_clock> verifier;

		void Load();
		void SaveDefault();

		void GenerateHash(std::string_view passwd, uint8_t* salt, uint8_t *digest);

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
		Ref<User> AddUser(std::string name, uint32_t userID, std::string passwd, UserAccessLevels accessLevel);
		Ref<User> AddUser(std::string name, uint32_t userID, uint8_t *hash, uint8_t *salt, UserAccessLevels accessLevel);

		size_t GetUserCount()
		{
			boost::shared_lock_guard lock(mutex);
			return userList.size();
		}
		Ref<User> GetUser(uint32_t userID);
		Ref<User> GetUserByName(std::string_view name);
		bool SetUserPassword(uint32_t userID, std::string_view passwd, std::string_view newPasswd);
		bool SetUserPassword(const Ref<User>& user, std::string_view passwd, std::string_view newPasswd);

		void RemoveUser(uint32_t userID);

		//JWT
		std::string GenerateToken(std::string_view name, std::string_view passwd);
		uint32_t VerifyToken(jwt::decoded_jwt& decoded);

		//IO
		void Save();
	};
}