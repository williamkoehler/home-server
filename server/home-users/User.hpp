#pragma once
#include "common.hpp"
#include "home-common/macros.hpp"

namespace server
{
    namespace users
    {
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
            const identifier_t id;
            const std::string name;
            uint8_t hash[SHA256_SIZE] = "";
            uint8_t salt[SALT_SIZE] = "";

            UserAccessLevel accessLevel;

          public:
            User(identifier_t id, const std::string& name, uint8_t hash[SHA256_SIZE], uint8_t salt[SALT_SIZE],
                 UserAccessLevel accessLevel);
            virtual ~User();
            static Ref<User> Create(identifier_t id, const std::string& name, uint8_t hash[SHA256_SIZE],
                                    uint8_t salt[SALT_SIZE], UserAccessLevel accessLevel);

            inline std::string GetName() const
            {
                return name;
            }

            inline uint32_t GetID() const
            {
                return id;
            }

            void GetHash(uint8_t* h);
            bool CompaireHash(uint8_t h[SHA256_SIZE]);
            bool SetHash(uint8_t h[SHA256_SIZE]);

            void GetSalt(uint8_t* s);

            UserAccessLevel GetAccessLevel();
            bool SetAccessLevel(UserAccessLevel v);

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
            void JsonSet(rapidjson::Value& input);
        };
    }
}
