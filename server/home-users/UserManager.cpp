#include "UserManager.hpp"
#include "User.hpp"
#include <cppcodec/base64_rfc4648.hpp>
#include <home-database/Database.hpp>
#include <openssl/rand.h>

#define AUTHKEY_SIZE 128
#ifndef NDEBUG
const uint64_t debugKey[] = {0x0123456789ABCDEF, 0xF0123456789ABCDE, 0xEF0123456789ABCD, 0xDEF0123456789ABC,
                             0xCDEF0123456789AB, 0xBCDEF0123456789A, 0xABCDEF0123456789, 0x9ABCDEF012345678,
                             0x89ABCDEF01234567, 0x789ABCDEF0123456, 0x6789ABCDEF012345, 0x56789ABCDEF01234,
                             0x456789ABCDEF0123, 0x3456789ABCDEF012, 0x23456789ABCDEF01, 0x123456789ABCDEF0};
#endif

namespace server
{
    namespace users
    {
        WeakRef<UserManager> instanceUserManager;

        UserManager::UserManager(const std::string& issuer) : issuer(issuer), verifier(jwt::default_clock())
        {
        }
        UserManager::~UserManager()
        {
            userList.clear();

            SAFE_DELETE_ARRAY(authenticationKey);
        }
        Ref<UserManager> UserManager::Create(const std::string& issuer)
        {
            if (!instanceUserManager.expired())
                return Ref<UserManager>(instanceUserManager);

            Ref<UserManager> userManager = boost::make_shared<UserManager>(issuer);
            instanceUserManager = userManager;

            // Generate authentication key
            userManager->authenticationKey = new uint8_t[AUTHKEY_SIZE];

#ifndef NDEBUG
            LOG_WARNING("In debug mode the JWT key will stay the same, this is a vulnerability.");
            memcpy(userManager->authenticationKey, debugKey, AUTHKEY_SIZE);
#else
            if (!RAND_bytes(userManager->authenticationKey, 128))
            {
                LOG_ERROR("Generate authentication key");
                return nullptr;
            }
#endif

            try
            {
                // Create JWT verifier
                userManager->verifier =
                    jwt::verify<JWTTraits>()
                        .allow_algorithm(jwt::algorithm::hs256(
                            std::string(reinterpret_cast<const char*>(userManager->authenticationKey), AUTHKEY_SIZE)))
                        .with_issuer(issuer);

                Ref<Database> database = Database::GetInstance();
                assert(database != nullptr);

                // Load users
                database->LoadUsers(boost::bind(&UserManager::LoadUser, userManager, boost::placeholders::_1,
                                                boost::placeholders::_2, boost::placeholders::_3,
                                                boost::placeholders::_4, boost::placeholders::_5));

                // Add default user if necessary
                if (database->GetUserCount() == 0)
                    userManager->AddUser("admin", "admin", UserAccessLevel::kAdministratorUserAccessLevel);
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

        bool UserManager::LoadUser(identifier_t id, const std::string& name, uint8_t hash[SHA256_DIGEST_LENGTH],
                                   uint8_t salt[SALT_SIZE], const std::string& accessLevel)
        {
            // Create user
            Ref<User> user = User::Create(id, name, hash, salt, ParseUserAccessLevel(accessLevel));

            // Add user
            if (user != nullptr)
            {
                userList[id] = user;

                return true;
            }
            else
                return false;
        }

        Ref<User> UserManager::AddUser(const std::string& name, const std::string& passwd, UserAccessLevel accessLevel)
        {
            // Generate random salt
            uint8_t salt[SALT_SIZE];
            if (!RAND_bytes(salt, SALT_SIZE))
            {
                LOG_ERROR("Generate salt");
                return nullptr;
            }

            // Calculate password hash
            uint8_t digest[SHA256_DIGEST_LENGTH];
            CalculateHash(passwd, salt, digest);

            return AddUser(name, digest, salt, accessLevel);
        }
        Ref<User> UserManager::AddUser(const std::string& name, uint8_t* hash, uint8_t* salt,
                                       UserAccessLevel accessLevel)
        {
            // Verify name
            if (!boost::regex_match(name, boost::regex(R"(^[a-zA-Z0-9_-]*$)")))
            {
                LOG_ERROR("Invalid user name '{0}' (does not match '[a-zA-Z0-9_-]*')", name);
                return nullptr;
            }

            // Check name
            if (boost::range::find_if(userList, [&name](const robin_hood::pair<const identifier_t, Ref<User>>& user)
                                      { return name == user.second->GetName(); }) != userList.end())
            {
                LOG_ERROR("User name already exists", name);
                return nullptr;
            }

            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Reserve user
            identifier_t id = database->ReserveUser();
            if (id == 0)
                return nullptr;

            // Update database
            if (!database->UpdateUser(id, name, hash, salt, StringifyUserAccessLevel(accessLevel)))
                return nullptr;

            // Create new user
            Ref<User> user = boost::make_shared<User>(id, name, hash, salt, accessLevel);

            // Add user
            if (user != nullptr)
                userList[user->GetID()] = user;
            else
            {
                database->RemoveUser(id);
                return nullptr;
            }

            return user;
        }

        Ref<User> UserManager::GetUser(identifier_t userID)
        {
            // boost::shared_lock_guard lock(mutex);

            const robin_hood::unordered_node_map<identifier_t, Ref<User>>::const_iterator it = userList.find(userID);
            if (it == userList.end())
                return nullptr;

            return (*it).second;
        }
        Ref<User> UserManager::GetUserByName(const std::string_view& name)
        {
            // boost::shared_lock_guard lock(mutex);

            const robin_hood::unordered_node_map<identifier_t, Ref<User>>::const_iterator it =
                boost::find_if(userList,
                               [&name](const robin_hood::pair<const identifier_t, Ref<User>>& pair) -> bool
                               { return pair.second->GetName() == name; });
            if (it == userList.end())
                return nullptr;

            return it->second;
        }
        Ref<User> UserManager::Authenticate(const std::string_view& name, const std::string_view& password)
        {
            // boost::shared_lock_guard lock(mutex);

            // Search for user
            const robin_hood::unordered_node_map<identifier_t, Ref<User>>::const_iterator it =
                boost::find_if(userList,
                               [&name](const robin_hood::pair<const identifier_t, Ref<User>>& pair) -> bool
                               { return pair.second->GetName() == name; });
            if (it == userList.end())
                return nullptr;

            Ref<User> user = it->second;
            assert(user != nullptr);

            // Check password
            uint8_t salt[SALT_SIZE];
            user->GetSalt(salt);

            uint8_t digest[SHA256_DIGEST_LENGTH];
            CalculateHash(password, salt, digest);

            // Compaire hash
            if (user->CompaireHash(digest))
                return user;
            else
                return nullptr;
        }

        bool UserManager::SetUserPassword(identifier_t userID, const std::string_view& passwd,
                                          const std::string_view& newPasswd)
        {
            // boost::shared_lock_guard lock(mutex);

            const robin_hood::unordered_node_map<identifier_t, Ref<User>>::const_iterator it = userList.find(userID);
            if (it == userList.end())
                return false;

            return SetUserPassword(it->second, passwd, newPasswd);
        }
        bool UserManager::SetUserPassword(const Ref<User>& user, const std::string_view& passwd,
                                          const std::string_view& newPasswd)
        {
            // Get salt
            uint8_t salt[SALT_SIZE];
            user->GetSalt(salt);

            // Verify password
            uint8_t digest[SHA256_DIGEST_LENGTH];
            CalculateHash(passwd, salt, digest);

            // Check hash
            if (!user->CompaireHash(digest))
                return false;

            // Generate and set new hash
            CalculateHash(newPasswd, salt, digest);
            user->SetHash(digest);

            return true;
        }

        bool UserManager::RemoveUser(identifier_t userID)
        {
            // Remove user
            if (userList.erase(userID))
            {
                Ref<Database> database = Database::GetInstance();
                assert(database != nullptr);

                // Remove user from database
                database->RemoveScriptSource(userID);

                return true;
            }
            else
                return false;
        }

        void UserManager::CalculateHash(const std::string_view& passwd, uint8_t* salt, uint8_t* digest)
        {
            // Create salted password
            std::string password = std::string(passwd.size() + SALT_SIZE, '\0');

            // Fill salted password
            memcpy((void*)password.data(), (void*)passwd.data(), passwd.size());         // Copy user password
            memcpy((void*)((uint8_t*)password.data() + passwd.size()), salt, SALT_SIZE); // Copy salt

            // Calculate sha256 hash
            SHA256_CTX context;
            SHA256_Init(&context);
            SHA256_Update(&context, password.data(), password.size());
            SHA256_Final(digest, &context);
        }

        // JWT
        std::string UserManager::GenerateJWTToken(const Ref<User>& user)
        {
            assert(user != nullptr);

            // boost::shared_lock_guard lock(mutex);

            return jwt::create<JWTTraits>()
                .set_issuer(issuer)
                .set_type("JWT")
                .set_payload_claim("id", user->GetID())
#ifndef _DEBUG
                .set_expires_at(jwt::date::clock::now() + std::chrono::hours(732))
#endif
                .sign(
                    jwt::algorithm::hs256(std::string(reinterpret_cast<const char*>(authenticationKey), AUTHKEY_SIZE)));
        }
        identifier_t UserManager::VerifyJWTToken(const std::string& token)
        {
            // boost::shared_lock_guard lock(mutex);

            try
            {
                jwt::decoded_jwt decoded = jwt::decode<JWTTraits>(token);

#ifndef _DEBUG
                if (!decoded.has_expires_at())
                    throw std::runtime_error("Invalid JWT");
#endif
                // Verify token using verifier
                verifier.verify(decoded); // Throws exception on fail

                return decoded.get_payload_claim("id").as_int();
            }
            catch (std::exception e)
            {
                return 0;
            }
        }
        void UserManager::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            // Lock main mutex
            // boost::shared_lock_guard lock(mutex);

            rapidjson::Value userListJson = rapidjson::Value(rapidjson::kArrayType);

            for (auto& [id, user] : userList)
            {
                rapidjson::Value userJson = rapidjson::Value(rapidjson::kObjectType);

                user->JsonGet(userJson, allocator);

                userListJson.PushBack(userJson, allocator);
            }

            output.AddMember("users", userListJson, allocator);
        }

        void UserManager::JsonSet(rapidjson::Value& input)
        {
            assert(input.IsObject());

            // Decode users if it exists
            rapidjson::Value::MemberIterator userListIt = input.FindMember("users");
            if (userListIt != input.MemberEnd() && userListIt->value.IsArray())
            {
                for (rapidjson::Value::ValueIterator userIt = userListIt->value.Begin();
                     userIt != userListIt->value.End(); userIt++)
                {
                    if (userIt->IsObject())
                    {
                        rapidjson::Value::MemberIterator userIDIt = input.FindMember("userid");
                        if (userIDIt == input.MemberEnd() || !userIDIt->value.IsUint())
                            return;

                        const robin_hood::unordered_node_map<identifier_t, Ref<User>>::const_iterator it =
                            userList.find(userIDIt->value.GetUint());
                        if (it != userList.end())
                            it->second->JsonSet(*userIt);
                    }
                }
            }
        }
    }
}