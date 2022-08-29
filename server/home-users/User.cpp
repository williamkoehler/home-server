#include "User.hpp"
#include "home-common/macros.hpp"
#include <home-database/Database.hpp>

namespace server
{
    namespace users
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

        User::User(identifier_t id, const std::string& name, uint8_t h[SHA256_SIZE], uint8_t s[SALT_SIZE],
                   UserAccessLevel accessLevel)
            : name(name), id(id), accessLevel(accessLevel)
        {
            memcpy(hash, h, SHA256_SIZE);
            memcpy(salt, s, SALT_SIZE);
        }
        User::~User()
        {
        }

        Ref<User> User::Create(identifier_t id, const std::string& name, uint8_t hash[SHA256_SIZE],
                               uint8_t salt[SALT_SIZE], UserAccessLevel accessLevel)
        {
            return boost::make_shared<User>(id, name, hash, salt, accessLevel);
        }

        std::string User::GetName()
        {
            boost::shared_lock_guard lock(mutex);
            return name;
        }
        bool User::SetName(const std::string& v)
        {
            boost::lock_guard lock(mutex);

            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Update database
            if (database->UpdateUserPropName(id, v))
            {
                // Assign new value
                name = v;

                return true;
            }

            return false;
        }

        void User::GetHash(uint8_t* h)
        {
            assert(h != nullptr);

            boost::shared_lock_guard lock(mutex);

            // Copy to output variable
            memcpy(h, hash, SHA256_SIZE);
        }
        bool User::CompaireHash(uint8_t h[SHA256_SIZE])
        {
            boost::shared_lock_guard lock(mutex);

            return memcmp(hash, h, SHA256_SIZE) == 0;
        }
        bool User::SetHash(uint8_t h[SHA256_SIZE])
        {
            boost::lock_guard lock(mutex);

            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Update database
            if (database->UpdateUserPropHash(id, h))
            {
                // Assign new value
                memcpy(hash, h, SHA256_SIZE);

                return true;
            }

            return false;
        }

        void User::GetSalt(uint8_t* s)
        {
            assert(s != nullptr);

            boost::shared_lock_guard lock(mutex);

            // Copy to output variable
            memcpy(s, salt, SALT_SIZE);
        }

        UserAccessLevel User::GetAccessLevel()
        {
            boost::shared_lock_guard lock(mutex);

            return accessLevel;
        }
        bool User::SetAccessLevel(UserAccessLevel v)
        {
            boost::lock_guard lock(mutex);

            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Update database
            if (database->UpdateUserPropAccessLevel(id, StringifyUserAccessLevel(v)))
            {
                // Assign new value
                accessLevel = v;

                return true;
            }
            return false;
        }

        void User::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            // Lock main mutex
            boost::shared_lock_guard lock(mutex);

            output.AddMember("id", rapidjson::Value(id), allocator);

            output.AddMember("name", rapidjson::Value(name.data(), name.size(), allocator), allocator);

            std::string accessLevelStr = StringifyUserAccessLevel(accessLevel);
            output.AddMember("accesslevel", rapidjson::Value(accessLevelStr.data(), accessLevelStr.size(), allocator),
                             allocator);
        }

        void User::JsonSet(rapidjson::Value& input)
        {
            assert(input.IsObject());

            // Lock main mutex
            boost::shared_lock_guard lock(mutex);

            rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
            if (nameIt != input.MemberEnd() && nameIt->value.IsString())
                SetName(std::string(nameIt->value.GetString(), nameIt->value.GetStringLength()));
        }
    }
}