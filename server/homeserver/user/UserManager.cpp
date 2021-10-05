#include "UserManager.hpp"
#include "User.hpp"
#include "../Core.hpp"
#include <openssl/rand.h>
#include <cppcodec/base64_rfc4648.hpp>

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

			userManager->Load();
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

	//Timestamp
	void UserManager::UpdateTimestamp()
	{
		const time_t ts = time(nullptr);
		timestamp = ts;
	}

	Ref<User> UserManager::AddUser(std::string name, uint32_t userID, std::string passwd, UserAccessLevels accessLevel)
	{
		uint8_t salt[SALT_LENGTH];
		if (!RAND_bytes(salt, SALT_LENGTH))
		{
			LOG_ERROR("Generate salt");
			return nullptr;
		}

		uint8_t digest[SHA256_DIGEST_LENGTH];
		GenerateHash(passwd, salt, digest);

		return AddUser(name, userID, digest, salt, accessLevel);
	}
	Ref<User> UserManager::AddUser(std::string name, uint32_t userID, uint8_t* hash, uint8_t* salt, UserAccessLevels accessLevel)
	{
		boost::lock_guard lock(mutex);

		uint32_t genID = userID ? userID : XXH32(name.c_str(), name.size(), 0x55534552);

		if (userList.count(genID))
		{
			LOG_ERROR("User ID already exists", name);
			return nullptr;
		}

		if (boost::range::find_if(userList, [&name](std::pair<uint32_t, Ref<User>> user) {return name == user.second->GetName(); }) != userList.end())
		{
			LOG_ERROR("User name already exists", name);
			return nullptr;
		}

		Ref<User> user = User::Create(name, genID, hash, salt, accessLevel);

		userList[genID] = user;

		UpdateTimestamp();

		return user;
	}

	Ref<User> UserManager::GetUser(uint32_t userID)
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

	bool UserManager::SetUserPassword(uint32_t userID, std::string_view passwd, std::string_view newPasswd)
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

	void UserManager::RemoveUser(uint32_t userID)
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
		Ref<User> user = GetUserByName(name);
		if (user == nullptr)
			throw std::invalid_argument("Name is incorrect");

		uint8_t salt[SALT_LENGTH];
		user->GetSalt(salt);

		uint8_t digest[SHA256_DIGEST_LENGTH];
		GenerateHash(passwd, salt, digest);

		if (user->CompaireHash(digest))
		{
			return jwt::create()
				.set_issuer(Core::GetInstance()->GetName())
				.set_type("JWT")
				.set_payload_claim("id", jwt::claim(picojson::value(static_cast<int64_t>(user->GetUserID()))))
#ifndef _DEBUG
				.set_expires_at(jwt::date::clock::now() + std::chrono::hours(732))
#endif
				.sign(jwt::algorithm::hs256(std::string(reinterpret_cast<const char*>(authenticationKey), AUTHKEY_SIZE)));
		}
		else
			throw std::invalid_argument("Password is incorrect");
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

	//IO
	void UserManager::Load()
	{
		LOG_INFO("Loading user information from 'user-info.json'");

		FILE* file = fopen("user-info.json", "r");
		if (file == nullptr)
		{
			LOG_ERROR("Failing to open/find 'user-info.json'");
			if (!boost::filesystem::exists("user-info.json"))
				SaveDefault();
			throw std::runtime_error("Open/find file 'user-info.json'");
		}

		char buffer[RAPIDJSON_BUFFER_SIZE_SMALL];
		rapidjson::FileReadStream stream(file, buffer, sizeof(buffer));

		rapidjson::Document document;
		if (document.ParseStream(stream).HasParseError() || !document.IsObject())
		{
			LOG_ERROR("Read 'user-info.json'");
			throw std::runtime_error("Read file 'users-info.json'");
		}

		//Read room list
		rapidjson::Value::MemberIterator userListIt = document.FindMember("user-list");
		if (userListIt == document.MemberEnd() || !userListIt->value.IsArray())
		{
			LOG_ERROR("Missing 'user-list' in 'user-info.json'");
			throw std::runtime_error("Invalid file 'user-info.json'");
		}

		rapidjson::Value& userListJson = userListIt->value;
		for (rapidjson::Value::ValueIterator userIt = userListJson.Begin(); userIt != userListJson.End(); userIt++)
		{
			if (!userIt->IsObject())
			{
				LOG_ERROR("Missing 'user-list' in 'user-info.json'");
				throw std::runtime_error("Invalid file 'user-info.json'");
			}

			//Read name
			rapidjson::Value::MemberIterator nameJson = userIt->FindMember("name");
			if (nameJson == userIt->MemberEnd() || !nameJson->value.GetString())
			{
				LOG_ERROR("Missing 'name' in user in 'user-list' in 'user-info.json'");
				throw std::runtime_error("Invalid file 'user-info.json'");
			}

			//Read id
			rapidjson::Value::MemberIterator userIDJson = userIt->FindMember("id");
			if (userIDJson == userIt->MemberEnd() || !userIDJson->value.GetUint())
			{
				LOG_ERROR("Missing 'id' in user in 'user-list' in 'user-info.json'");
				throw std::runtime_error("Invalid file 'user-info.json'");
			}

			//Read hash
			rapidjson::Value::MemberIterator hashJson = userIt->FindMember("hash");
			if (hashJson == userIt->MemberEnd() || !hashJson->value.GetString())
			{
				LOG_ERROR("Missing 'hash' in user in 'user-list' in 'user-info.json'");
				throw std::runtime_error("Invalid file 'user-info.json'");
			}
			std::vector<uint8_t> hash = cppcodec::base64_rfc4648::decode(hashJson->value.GetString(), hashJson->value.GetStringLength());
			if (hash.size() != SHA256_DIGEST_LENGTH)
			{
				LOG_ERROR("Invalid encoded 'hash' in user in 'user-list' in 'user-info.json'");
				throw std::runtime_error("Invalid file 'user-info.json'");
			}

			//Read hash
			rapidjson::Value::MemberIterator saltJson = userIt->FindMember("salt");
			if (saltJson == userIt->MemberEnd() || !saltJson->value.GetString())
			{
				LOG_ERROR("Missing 'salt' in user in 'user-list' in 'user-info.json'");
				throw std::runtime_error("Invalid file 'user-info.json'");
			}
			std::vector<uint8_t> salt = cppcodec::base64_rfc4648::decode(saltJson->value.GetString(), saltJson->value.GetStringLength());
			if (salt.size() != SALT_LENGTH)
			{
				LOG_ERROR("Invalid encoded 'salt' in user in 'user-list' in 'user-info.json'");
				throw std::runtime_error("Invalid file 'user-info.json'");
			}

			//Read access level
			rapidjson::Value::MemberIterator accessLevelJson = userIt->FindMember("access-level");
			if (accessLevelJson == userIt->MemberEnd() || !accessLevelJson->value.GetUint())
			{
				LOG_ERROR("Missing 'access-level' in user in 'user-list' in 'user-info.json'");
				throw std::runtime_error("Invalid file 'user-info.json'");
			}
			uint32_t accessLevel = accessLevelJson->value.GetUint();
			if (accessLevel < 0 || accessLevel > 2)
			{
				accessLevel = static_cast<uint32_t>(UserAccessLevels::kRestrictedUserAccessLevel);
				LOG_WARNING("Invalid user 'access-level'. Access level is a number between 0 and 2. \n0 = Restricted\n1 = Normal\n2 = Operator.");
			}

			const std::string name(nameJson->value.GetString(), nameJson->value.GetStringLength());

			Ref<User> user = AddUser(
				name,
				userIDJson->value.GetUint(),
				hash.data(),
				salt.data(),
				static_cast<UserAccessLevels>(accessLevel));
			if (user == nullptr)
			{
				LOG_ERROR("Add user {0}", name);
				throw std::runtime_error("Add user");
			}
		}

		fclose(file);
	}
	void UserManager::SaveDefault()
	{
		LOG_INFO("Saving default user information to file");

		//Create json
		rapidjson::Document document = rapidjson::Document(rapidjson::kObjectType);

		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		//Create user list
		rapidjson::Value userListJson = rapidjson::Value(rapidjson::kArrayType);

		//Add default user
		{
			rapidjson::Value userJson = rapidjson::Value(rapidjson::kObjectType);

			userJson.AddMember("name", rapidjson::Value("admin", 5, allocator), allocator);
			userJson.AddMember("id", rapidjson::Value(1234), allocator);

			uint8_t s[SALT_LENGTH];
			if (!RAND_bytes(s, SALT_LENGTH))
				memcpy(s, "adminadminadmina", 16);

			uint8_t digest[SHA256_DIGEST_LENGTH];
			GenerateHash("home", s, digest);

			{

				std::string hash = cppcodec::base64_rfc4648::encode(digest, SHA256_DIGEST_LENGTH);
				userJson.AddMember("hash", rapidjson::Value(hash.c_str(), hash.size(), allocator), allocator);
			}
			{
				std::string salt = cppcodec::base64_rfc4648::encode(s, SALT_LENGTH);
				userJson.AddMember("salt", rapidjson::Value(salt.c_str(), salt.size(), allocator), allocator);
			}
			userJson.AddMember("access-level", rapidjson::Value(static_cast<int64_t>(UserAccessLevels::kAdministratorUserAccessLevel)), allocator);

			userListJson.PushBack(userJson, allocator);
		}

		document.AddMember("user-list", userListJson, allocator);

		//Save to file
		FILE* file = fopen("user-info.json", "w");
		if (file == nullptr)
		{
			LOG_ERROR("Open/find 'user-info.json'");
			throw std::runtime_error("Open/find file 'user-info.json'");
		}

		char buffer[RAPIDJSON_BUFFER_SIZE_SMALL];
		rapidjson::FileWriteStream stream(file, buffer, sizeof(buffer));

		rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer = rapidjson::PrettyWriter<rapidjson::FileWriteStream>(stream);
		document.Accept(writer);

		fclose(file);
	}
	void UserManager::Save()
	{
		boost::shared_lock_guard lock(mutex);

		LOG_INFO("Saving user information to file");

		//Create json
		rapidjson::Document document = rapidjson::Document(rapidjson::kObjectType);

		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		//Create user list
		rapidjson::Value userListJson = rapidjson::Value(rapidjson::kArrayType);
		for (std::pair<uint32_t, Ref<User>> pair : userList)
		{
			boost::shared_lock_guard lock(pair.second->mutex);

			rapidjson::Value userJson = rapidjson::Value(rapidjson::kObjectType);

			userJson.AddMember("name", rapidjson::Value(pair.second->name.c_str(), pair.second->name.size(), allocator), allocator);
			userJson.AddMember("id", rapidjson::Value(pair.first), allocator);

			{
				std::string hash = cppcodec::base64_rfc4648::encode(pair.second->hash, SHA256_DIGEST_LENGTH);
				userJson.AddMember("hash", rapidjson::Value(hash.c_str(), hash.size(), allocator), allocator);
			}
			{
				std::string salt = cppcodec::base64_rfc4648::encode(pair.second->salt, SALT_LENGTH);
				userJson.AddMember("salt", rapidjson::Value(salt.c_str(), salt.size(), allocator), allocator);
			}
			userJson.AddMember("access-level", rapidjson::Value(static_cast<int64_t>(pair.second->accessLevel)), allocator);

			userListJson.PushBack(userJson, allocator);
		}

		document.AddMember("user-list", userListJson, allocator);

		//Save to file
		FILE* file = fopen("user-info.json", "w");
		if (file == nullptr)
		{
			LOG_ERROR("Open/find 'user-info.json'");
			throw std::runtime_error("Open/find file 'user-info.json'");
		}

		char buffer[RAPIDJSON_BUFFER_SIZE_SMALL];
		rapidjson::FileWriteStream stream(file, buffer, sizeof(buffer));

		rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer = rapidjson::PrettyWriter<rapidjson::FileWriteStream>(stream);
		document.Accept(writer);

		fclose(file);
	}
}