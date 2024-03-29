#pragma once
#include "../../../homeserver/user/user.hpp"
#include "../../common.hpp"

static uint8_t sha256_empty[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t sha256_01[] = {0xb7, 0x6d, 0xfc, 0x71, 0x99, 0xb6, 0x17, 0x85, 0xaa, 0x18, 0x4b,
                              0xa9, 0x55, 0x7b, 0x95, 0x1e, 0x2d, 0x75, 0x6c, 0x0d, 0x33, 0x09,
                              0xb9, 0xcb, 0xb3, 0x80, 0x38, 0xb8, 0xbc, 0x99, 0x40, 0xc5};
static uint8_t sha256_02[] = {0xf8, 0x08, 0x63, 0xcd, 0xc2, 0x95, 0xc6, 0x9f, 0xe9, 0x4b, 0xdf,
                              0x4e, 0xd6, 0xa6, 0x22, 0x13, 0x2f, 0x43, 0x38, 0x48, 0x2e, 0x0d,
                              0xdf, 0x99, 0x77, 0x34, 0x9d, 0xd8, 0xc1, 0x78, 0x3e, 0xf6};
static uint8_t sha256_03[] = {0x3e, 0x4f, 0x8b, 0xf2, 0x9b, 0x64, 0x73, 0xb8, 0x44, 0xf2, 0x74,
                              0xae, 0x17, 0xd2, 0xf0, 0x78, 0x1d, 0x4a, 0x01, 0xb8, 0x72, 0x53,
                              0x04, 0x02, 0x31, 0x48, 0x1a, 0x27, 0xcf, 0x63, 0x80, 0x11};
static uint8_t sha256_04[] = {0xb2, 0x9a, 0x28, 0x86, 0xf2, 0x0a, 0xc6, 0x7d, 0x56, 0x2b, 0x16,
                              0x72, 0xe0, 0x91, 0x84, 0x2d, 0x84, 0xb9, 0x63, 0x04, 0xa7, 0x9c,
                              0xd9, 0x3f, 0x71, 0x96, 0xb4, 0x01, 0x14, 0x32, 0xa8, 0x6f};
static uint8_t sha256_05[] = {0x64, 0xf2, 0x96, 0xae, 0x28, 0xfa, 0x1c, 0xae, 0xf2, 0xea, 0x76,
                              0x7e, 0x5e, 0x7a, 0xf0, 0xb3, 0xdc, 0xc5, 0xc5, 0x35, 0xe0, 0x03,
                              0x92, 0x68, 0xd6, 0xe2, 0x5c, 0xc4, 0xbe, 0x9f, 0x5c, 0x53};
static uint8_t sha256_06[] = {0xfa, 0x1c, 0x32, 0x64, 0x02, 0x86, 0xc0, 0x0e, 0xf9, 0xd5, 0x55,
                              0xad, 0xc8, 0x22, 0x77, 0xc6, 0xe8, 0x15, 0xad, 0xea, 0x1c, 0x63,
                              0xb3, 0xb2, 0xaa, 0x63, 0x97, 0x54, 0xce, 0xaa, 0x8e, 0xaa};
static uint8_t sha256_07[] = {0x13, 0xf8, 0x70, 0x27, 0x00, 0xbe, 0xf3, 0x73, 0xf2, 0xf5, 0x4c,
                              0x1d, 0xb7, 0x48, 0x36, 0x06, 0xa5, 0xd7, 0xed, 0xb5, 0xb8, 0x42,
                              0x78, 0x23, 0x05, 0x17, 0x4a, 0xd7, 0xd3, 0x81, 0x01, 0xd1};
static uint8_t sha256_08[] = {0xc4, 0xa7, 0xc9, 0x85, 0x53, 0xad, 0xd9, 0x2e, 0xcb, 0x71, 0xa7,
                              0xf9, 0xa5, 0x0b, 0xbc, 0xe4, 0x91, 0x72, 0x7b, 0x5d, 0x4a, 0xc3,
                              0x60, 0x5d, 0x7b, 0xd9, 0x06, 0x01, 0x26, 0xb4, 0x89, 0x99};
static uint8_t sha256_09[] = {0x7d, 0x40, 0x3e, 0x86, 0x20, 0x1c, 0x9a, 0x45, 0x00, 0x64, 0x92,
                              0xf9, 0x09, 0x74, 0xc0, 0x87, 0x54, 0x0f, 0x5f, 0xe1, 0x72, 0xf2,
                              0x7e, 0xe0, 0x1a, 0x58, 0x9c, 0xa9, 0xf5, 0x49, 0x20, 0xa0};
static uint8_t sha256_10[] = {0xb2, 0x54, 0xd5, 0x71, 0x00, 0xd6, 0x38, 0xff, 0x11, 0x60, 0x15,
                              0x75, 0xdd, 0xa2, 0xaf, 0xb0, 0x4b, 0xfa, 0xe3, 0x58, 0x59, 0xd1,
                              0x7c, 0x94, 0xa3, 0x9e, 0x2f, 0x13, 0x2a, 0x3b, 0x4a, 0xa6};

static uint8_t salt_empty[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t salt_01[] = {0x09, 0xfc, 0x71, 0x99, 0xb6, 0xbc, 0x99, 0x40,
                            0xc5, 0x33, 0xb9, 0xcb, 0xb7, 0x6d, 0x38, 0xb8};
static uint8_t salt_02[] = {0x0d, 0x63, 0xcd, 0xc2, 0x95, 0xc1, 0x78, 0x3e,
                            0xf6, 0x2e, 0xdf, 0x99, 0xf8, 0x08, 0x9d, 0xd8};
static uint8_t salt_03[] = {0x53, 0x8b, 0xf2, 0x9b, 0x64, 0xcf, 0x63, 0x80,
                            0x11, 0x72, 0x04, 0x02, 0x3e, 0x4f, 0x1a, 0x27};
static uint8_t salt_04[] = {0x9c, 0x28, 0x86, 0xf2, 0x0a, 0x14, 0x32, 0xa8,
                            0x6f, 0xa7, 0xd9, 0x3f, 0xb2, 0x9a, 0xb4, 0x01};
static uint8_t salt_05[] = {0x03, 0x96, 0xae, 0x28, 0xfa, 0xbe, 0x9f, 0x5c,
                            0x53, 0xe0, 0x92, 0x68, 0x64, 0xf2, 0x5c, 0xc4};
static uint8_t salt_06[] = {0x63, 0x32, 0x64, 0x02, 0x86, 0xce, 0xaa, 0x8e,
                            0xaa, 0x1c, 0xb3, 0xb2, 0xfa, 0x1c, 0x97, 0x54};
static uint8_t salt_07[] = {0x42, 0x70, 0x27, 0x00, 0xbe, 0xd3, 0x81, 0x01,
                            0xd1, 0xb8, 0x78, 0x23, 0x13, 0xf8, 0x4a, 0xd7};
static uint8_t salt_08[] = {0xc3, 0xc9, 0x85, 0x53, 0xad, 0x26, 0xb4, 0x89,
                            0x99, 0x4a, 0x60, 0x5d, 0xc4, 0xa7, 0x06, 0x01};
static uint8_t salt_09[] = {0xf2, 0x3e, 0x86, 0x20, 0x1c, 0xf5, 0x49, 0x20,
                            0xa0, 0x72, 0x7e, 0xe0, 0x7d, 0x40, 0x9c, 0xa9};
static uint8_t salt_10[] = {0xd1, 0xd5, 0x71, 0x00, 0xd6, 0x2a, 0x3b, 0x4a,
                            0xa6, 0x59, 0x7c, 0x94, 0xb2, 0x54, 0x2f, 0x13};

struct UserCache
{
    identifier_t id;
    std::string name;
    uint8_t* hash;
    uint8_t* salt;
    server::UserAccessLevel accessLevel;
};
void TestTableSize(size_t tableSize)
{
    //! Check table size
    BOOST_CHECK_MESSAGE(database->GetUserCount() == tableSize, "User table has too many or too few elements");
}
void TestTableContent(const UserCache** users, size_t userCount)
{
    size_t counter = 0;

    // Iterate over every element in the database
    BOOST_REQUIRE_MESSAGE(database->LoadUsers([&users, &userCount,
                                               &counter](identifier_t id, const std::string& name,
                                                         uint8_t hash[SHA256_DIGEST_LENGTH], uint8_t salt[SALT_LENGTH],
                                                         server::UserAccessLevel accessLevel) -> void {
        if (counter < userCount)
        {
            const UserCache* user = users[counter];

            //! Check element id
            BOOST_CHECK_MESSAGE(id == user->id, "ID is not correct: '" << id << "' != '" << user->id << "'");

            //! Check element name
            BOOST_CHECK_MESSAGE(name == user->name, "Name is not correct: '" << name << "' != '" << user->name << "'");

            //! Check element hash
            BOOST_CHECK_MESSAGE(memcmp(hash, user->hash, SHA256_DIGEST_LENGTH) == 0, "Hash is not correct");

            //! Check element salt
            BOOST_CHECK_MESSAGE(memcmp(salt, user->salt, SALT_LENGTH) == 0, "Salt is not correct");

            //! Check element access level
            BOOST_CHECK_MESSAGE(accessLevel == user->accessLevel,
                                "Access level is not correct: '"
                                    << server::StringifyUserAccessLevel(accessLevel) << "' != '"
                                    << server::StringifyUserAccessLevel(user->accessLevel) << "'");
        }

        counter++;
    }),
                          "Load users from database");

    //! Compaire element count
    BOOST_CHECK_MESSAGE(counter <= userCount, "User table has too many elements");
    BOOST_CHECK_MESSAGE(counter >= userCount, "User table has too few elements");
}

#define UNKNOWN_USER "unknown user"

static UserCache user01 = {1, UNKNOWN_USER, sha256_empty, salt_empty,
                           server::UserAccessLevel::kRestrictedUserAccessLevel};
static UserCache user02 = {2, UNKNOWN_USER, sha256_empty, salt_empty,
                           server::UserAccessLevel::kRestrictedUserAccessLevel};
static UserCache user03 = {3, UNKNOWN_USER, sha256_empty, salt_empty,
                           server::UserAccessLevel::kRestrictedUserAccessLevel};
static UserCache user04 = {4, UNKNOWN_USER, sha256_empty, salt_empty,
                           server::UserAccessLevel::kRestrictedUserAccessLevel};
static UserCache user05 = {5, UNKNOWN_USER, sha256_empty, salt_empty,
                           server::UserAccessLevel::kRestrictedUserAccessLevel};
static UserCache user06 = {6, UNKNOWN_USER, sha256_empty, salt_empty,
                           server::UserAccessLevel::kRestrictedUserAccessLevel};
static UserCache user07 = {7, UNKNOWN_USER, sha256_empty, salt_empty,
                           server::UserAccessLevel::kRestrictedUserAccessLevel};
static UserCache user08 = {8, UNKNOWN_USER, sha256_empty, salt_empty,
                           server::UserAccessLevel::kRestrictedUserAccessLevel};
static UserCache user09 = {9, UNKNOWN_USER, sha256_empty, salt_empty,
                           server::UserAccessLevel::kRestrictedUserAccessLevel};
static UserCache user10 = {10, UNKNOWN_USER, sha256_empty, salt_empty,
                           server::UserAccessLevel::kRestrictedUserAccessLevel};

static UserCache* userList[] = {&user01, &user02, &user03, &user04, &user05,
                                &user06, &user07, &user08, &user09, &user10};

void UpdateUser(identifier_t id, const std::string& name, uint8_t* hash, uint8_t* salt,
                server::UserAccessLevel accessLevel)
{
    assert(id > 0);

    //! Update user cache
    UserCache* userCache = userList[id - 1];
    userCache->id = id;
    userCache->name = name;
    userCache->hash = hash;
    userCache->salt = salt;
    userCache->accessLevel = accessLevel;

    //! Create user instance
    Ref<server::User> user = server::User::Create(name, id, hash, salt, accessLevel);
    BOOST_REQUIRE_MESSAGE(user != nullptr, "Create server::User instance");

    BOOST_CHECK_MESSAGE(database->UpdateUser(user) == true, "Update user '" << name << "'");

    //! Destroy user instance
    user = nullptr;
}
void UpdateUserName(identifier_t id, const std::string& name)
{
    assert(id > 0);

    //! Update element list
    UserCache* userCache = userList[id - 1];
    userCache->id = id;
    userCache->name = name;

    //! Create user instance
    Ref<server::User> user = server::User::Create(userCache->name, userCache->id, sha256_empty, salt_empty,
                                                  server::UserAccessLevel::kRestrictedUserAccessLevel);
    BOOST_REQUIRE_MESSAGE(user != nullptr, "Create server::User instance");

    BOOST_CHECK_MESSAGE(database->UpdateUserPropName(user, "old name", name) == true, "Update user prop name");

    //! Destroy user instance
    user = nullptr;
}
void UpdateUserAccessLevel(identifier_t id, server::UserAccessLevel accessLevel)
{
    assert(id > 0);

    //! Update element list
    UserCache* userCache = userList[id - 1];
    userCache->id = id;
    userCache->accessLevel = accessLevel;

    //! Create user instance
    Ref<server::User> user = server::User::Create(userCache->name, userCache->id, sha256_empty, salt_empty,
                                                  server::UserAccessLevel::kRestrictedUserAccessLevel);
    BOOST_REQUIRE_MESSAGE(user != nullptr, "Create server::User instance");

    BOOST_CHECK_MESSAGE(database->UpdateUserPropAccessLevel(user, server::UserAccessLevel::kRestrictedUserAccessLevel,
                                                            accessLevel) == true,
                        "Update user prop access level");

    //! Destroy user instance
    user = nullptr;
}
void UpdateUserHash(identifier_t id, uint8_t* hash)
{
    assert(id > 0);

    //! Update element list
    UserCache* userCache = userList[id - 1];
    userCache->id = id;
    userCache->hash = hash;

    //! Create user instance
    Ref<server::User> user = server::User::Create(userCache->name, userCache->id, sha256_empty, salt_empty,
                                                  server::UserAccessLevel::kRestrictedUserAccessLevel);
    BOOST_REQUIRE_MESSAGE(user != nullptr, "Create server::User instance");

    BOOST_CHECK_MESSAGE(database->UpdateUserPropHash(user, hash) == true, "Update user prop hash");

    //! Destroy user instance
    user = nullptr;
}
void UpdateUserSalt(identifier_t id, uint8_t* salt)
{
    assert(id > 0);

    //! Update element list
    UserCache* userCache = userList[id - 1];
    userCache->id = id;
    userCache->salt = salt;

    //! Create user instance
    Ref<server::User> user = server::User::Create(userCache->name, userCache->id, sha256_empty, salt_empty,
                                                  server::UserAccessLevel::kRestrictedUserAccessLevel);
    BOOST_REQUIRE_MESSAGE(user != nullptr, "Create server::User instance");

    BOOST_CHECK_MESSAGE(database->UpdateUserPropSalt(user, salt) == true, "Update user prop salt");

    //! Destroy user instance
    user = nullptr;
}