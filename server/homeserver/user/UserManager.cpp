#include "UserManager.hpp"
#include "User.hpp"
#include "../Core.hpp"
#include <openssl/rand.h>
#include <cppcodec/base64_rfc4648.hpp>
#include "../database/Database.hpp"

#define AUTHKEY_SIZE 128
#ifdef _DEBUG
const uint64_t debugKey[] = {
	0x0123456789ABCDEF,
	0xF0123456789ABCDE,
	0xEF0123456789ABCD,
	0xDEF0123456789ABC,
	0xCDEF0123456789AB,
	0xBCDEF0123456789A,
	0xABCDEF0123456789,
	0x9ABCDEF012345678,
	0x89ABCDEF01234567,
	0x789ABCDEF0123456,
	0x6789ABCDEF012345,
	0x56789ABCDEF01234,
	0x456789ABCDEF0123,
	0x3456789ABCDEF012,
	0x23456789ABCDEF01,
	0x123456789ABCDEF0 };
#endif

namespace server
{
	boost::weak_ptr<UserManager> instanceUserManager;

	UserManager::UserManager()
		: verifier(jwt::default_clock())
	{
	}
	UserManager::~UserManager()
	{
		userList.clear();

		SAFE_DELETE_ARRAY(authenticationKey);
	}
	Ref<UserManager> UserManager::Create()
	{
		if (!instanceUserManager.expired())
			return Ref<UserManager>(instanceUserManager);

		Ref<UserManager> userManager = boost::make_shared<UserManager>();
		instanceUserManager = userManager;

		//Generate authentication key
		userManager->authenticationKey = new uint8_t[AUTHKEY_SIZE];

#ifdef _DEBUG
		LOG_WARNING("In debug mode the JWT key will stay the same, this is a vulnerability.");
		memcpy(userManager->authenticationKey, debugKey, AUTHKEY_SIZE);
#else
		if (!RAND_bytes(userManager->authenticationKey, 128))
		{
			userManager = nullptr;
			LOG_ERROR("Generate authentication key");
			return nullptr;
		}
#endif

		try
		{
			//Create JWT verifier
			userManager->verifier = jwt::verify()
				.allow_algorithm(jwt::algorithm::hs256(std::string(reinterpret_cast<const char*>(userManager->authenticationKey), AUTHKEY_SIZE)))
				.with_issuer(Core::GetInstance()->GetName());

			Ref<Database> database = Database::GetInstance();
			assert(database != nullptr);

			// Load users
			database->LoadUsers(
				boost::bind(&UserManager::LoadUser, userManager,
					boost::placeholders::_1,
					boost::placeholders::_2,
					boost::placeholders::_3,
					boost::placeholders::_4,
					boost::placeholders::_5));

			// Check for default user
			if (database->GetUserCount() == 0)
				userManager->AddUser("admin", "admin", server::UserAccessLevel::kAdministratorUserAccessLevel);
		}
		catch (std::exception e)
		{
			return nullptr;
		}

		userManager->UpdateTimestamp();

		return userManager;
	}
	Ref<UserManager> UserManager::GetInstance()
	{
		return Ref<UserManager>(instanceUserManager);
	}

	// Timestamp
	void UserManager::UpdateTimestamp()
	{
		const time_t ts = time(nullptr);
		timestamp = ts;
	}

	bool UserManager::LoadUser(identifier_t userID, const std::string& name, uint8_t hash[SHA256_DIGEST_LENGTH], uint8_t salt[SALT_LENGTH], UserAccessLevel accessLevel)
	{
		// Create new user
		Ref<User> user = boost::make_shared<User>(name, userID, hash, salt, accessLevel);

		if (user != nullptr)
		{
			userList[userID] = user;

			return true;
		}
		else
			return false;
	}

	Ref<User> UserManager::AddUser(std::string name, std::string passwd, UserAccessLevel accessLevel)
	{
		uint8_t salt[SALT_LENGTH];
		if (!RAND_bytes(salt, SALT_LENGTH))
		{
			LOG_ERROR("Generate salt");
			return nullptr;
		}

		uint8_t digest[SHA256_DIGEST_LENGTH];
		GenerateHash(passwd, salt, digest);

		return AddUser(name, digest, salt, accessLevel);
	}
	Ref<User> UserManager::AddUser(std::string name, uint8_t* hash, uint8_t* salt, UserAccessLevel accessLevel)
	{
		boost::lock_guard lock(mutex);

		Ref<Database> database = Database::GetInstance();
		assert(database != nullptr);

		// Reserve room in database
		identifier_t userID = database->ReserveUser();
		if (userID == 0)
			return nullptr;

		if (boost::range::find_if(userList, [&name](std::pair<uint32_t, Ref<User>> user) {return name == user.second->GetName(); }) != userList.end())
		{
			LOG_ERROR("User name already exists", name);
			return nullptr;
		}

		Ref<User> user = boost::make_shared<User>(name, userID, hash, salt, accessLevel);

		if (user != nullptr)
		{
			//user->Load(json);

			if (!database->UpdateUser(user))
				return nullptr;

			userList[user->GetUserID()] = user;
		}

		return user;
	}

	Ref<User> UserManager::GetUser(identifier_t userID)
	{
		boost::shared_lock_guard lock(mutex);

		const boost::unordered::unordered_map<uint32_t, Ref<User>>::const_iterator it = userList.find(userID);
		if (it == userList.end())
			return nullptr;

		return (*it).second;
	}
	Ref<User> UserManager::GetUserByName(std::string_view name)
	{
		boost::shared_lock_guard lock(mutex);

		const boost::unordered::unordered_map<uint32_t, Ref<User>>::const_iterator it = boost::find_if(userList, [&name](std::pair<uint32_t, Ref<User>> pair) -> bool
			{ return pair.second->GetName() == name; });
		if (it == userList.end())
			return nullptr;

		return (*it).second;
	}
	Ref<User> UserManager::GetUserByPassword(std::string_view name, std::string_view password)
	{
		boost::shared_lock_guard lock(mutex);

		const boost::unordered::unordered_map<uint32_t, Ref<User>>::const_iterator it = boost::find_if(userList, [&name](std::pair<uint32_t, Ref<User>> pair) -> bool
			{ return pair.second->GetName() == name; });
		if (it == userList.end())
			return nullptr;

		Ref<User> user = it->second;
		assert(user != nullptr);

		// Check password
		uint8_t salt[SALT_LENGTH];
		user->GetSalt(salt);

		uint8_t digest[SHA256_DIGEST_LENGTH];
		GenerateHash(password, salt, digest);

		// Compaire hash
		if (user->CompaireHash(digest))
			return user;
		else
			return nullptr;
	}

	bool UserManager::SetUserPassword(identifier_t userID, std::string_view passwd, std::string_view newPasswd)
	{
		boost::shared_lock_guard lock(mutex);

		const boost::unordered::unordered_map<uint32_t, Ref<User>>::const_iterator it = userList.find(userID);
		if (it == userList.end())
			return false;

		return SetUserPassword(it->second, passwd, newPasswd);
	}
	bool UserManager::SetUserPassword(const Ref<User>& user, std::string_view passwd, std::string_view newPasswd)
	{
		//Get salt
		uint8_t salt[SALT_LENGTH];
		user->GetSalt(salt);

		//Verify password
		uint8_t digest[SHA256_DIGEST_LENGTH];
		GenerateHash(passwd, salt, digest);

		//Check hash
		if (!user->CompaireHash(digest))
			return false;

		//Generate and set new hash
		GenerateHash(newPasswd, salt, digest);
		user->SetHash(digest);

		return true;
	}

	void UserManager::RemoveUser(identifier_t userID)
	{
		boost::lock_guard lock(mutex);

		const boost::unordered::unordered_map<uint32_t, Ref<User>>::const_iterator it = userList.find(userID);
		if (it == userList.end())
			throw std::invalid_argument("User ID does not exist");

		userList.erase(it);

		UpdateTimestamp();
	}

	void UserManager::GenerateHash(std::string_view passwd, uint8_t* salt, uint8_t* digest)
	{
		//Create salted password
		const size_t passwordLength = passwd.size() + SALT_LENGTH;
		char* password = new char[passwordLength];

		//Fill salted password
		memcpy(password, passwd.data(), passwd.size());
		memcpy(password + passwd.size(), salt, SALT_LENGTH);

		SHA256_CTX context;
		SHA256_Init(&context);
		SHA256_Update(&context, password, passwordLength);
		SHA256_Final(digest, &context);

		SAFE_DELETE_ARRAY(password);
	}

	//JWT
	std::string UserManager::GenerateToken(std::string_view name, std::string_view passwd)
	{
		boost::shared_lock_guard lock(mutex);

		//Get user
		Ref<User> user = GetUserByPassword(name, passwd);
		if (user == nullptr)
			throw std::invalid_argument("Name or password is incorrect");

		return jwt::create()
			.set_issuer(Core::GetInstance()->GetName())
			.set_type("JWT")
			.set_payload_claim("id", jwt::claim(picojson::value(static_cast<int64_t>(user->GetUserID()))))
#ifndef _DEBUG
			.set_expires_at(jwt::date::clock::now() + std::chrono::hours(732))
#endif
			.sign(jwt::algorithm::hs256(std::string(reinterpret_cast<const char*>(authenticationKey), AUTHKEY_SIZE)));
	}
	uint32_t UserManager::VerifyToken(jwt::decoded_jwt& decoded)
	{
		boost::shared_lock_guard lock(mutex);

#ifndef _DEBUG
		if (!decoded.has_expires_at())
			throw std::runtime_error("Invalid JWT");
#endif
		//Verify token using verifier
		verifier.verify(decoded); // Throws exception on fail

		return decoded.get_payload_claim("id").as_int();
	}
}