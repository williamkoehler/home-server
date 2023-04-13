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
            : id(id), name(name), accessLevel(accessLevel)
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

        void User::GetHash(uint8_t* h)
        {
            assert(h != nullptr);

            // Copy to output variable
            memcpy(h, hash, SHA256_SIZE);
        }
        bool User::CompaireHash(uint8_t h[SHA256_SIZE])
        {
            return memcmp(hash, h, SHA256_SIZE) == 0;
        }
        bool User::SetHash(uint8_t h[SHA256_SIZE])
        {
            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Update database
            if (database->UpdateUserHash(id, h, salt))
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

            // Copy to output variable
            memcpy(s, salt, SALT_SIZE);
        }

        UserAccessLevel User::GetAccessLevel()
        {
            return accessLevel;
        }
        bool User::SetAccessLevel(UserAccessLevel v)
        {
            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Update database
            if (database->UpdateUserAccessLevel(id, StringifyUserAccessLevel(v)))
            {
                // Assign new value
                accessLevel = v;

                return true;
            }
            return false;
        }

        void User::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) const
        {
            assert(output.IsObject());

            output.AddMember("id", rapidjson::Value(id), allocator);

            output.AddMember("name", rapidjson::Value(name.data(), name.size(), allocator), allocator);

            std::string accessLevelStr = StringifyUserAccessLevel(accessLevel);
            output.AddMember("accesslevel", rapidjson::Value(accessLevelStr.data(), accessLevelStr.size(), allocator),
                             allocator);
        }

        void User::JsonSet(const rapidjson::Value& input)
        {
            assert(input.IsObject());
        }
    }
}