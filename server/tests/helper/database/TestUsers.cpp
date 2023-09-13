#include "TestUsers.hpp"
#include "../mock/MockString.hpp"
#include "Testdatabase.hpp"

BOOST_AUTO_TEST_CASE(check_initial_state)
{
    BOOST_REQUIRE(database != nullptr);

    // Check initial table state
    TestTableSize(0);
    TestTableContent(nullptr, 0);
}

//! Test reserve
BOOST_AUTO_TEST_CASE(test_reserve_user)
{
    // Insert 10 different users
    database->ReserveUser();
    database->ReserveUser();
    database->ReserveUser();
    database->ReserveUser();
    database->ReserveUser();

    database->ReserveUser();
    database->ReserveUser();
    database->ReserveUser();
    database->ReserveUser();
    database->ReserveUser();

    // Check table
    TestTableContent(nullptr, 0);

    // Check table size
    TestTableSize(10);
}

//! Test update
BOOST_AUTO_TEST_CASE(test_partial_update_user_1)
{
    static const UserCache* users[] = {
        &user01, &user02, &user04, &user05, &user09,
    };

    // Update 5 users
    UpdateUser(&user01, "resU 10", sha256_02, salt_02, server::UserAccessLevel::kAdministratorUserAccessLevel);
    UpdateUser(&user02, "resU 20", sha256_04, salt_04, server::UserAccessLevel::kMaintainerUserAccessLevel);
    UpdateUser(&user04, "resU 40", sha256_05, salt_05, server::UserAccessLevel::kAdministratorUserAccessLevel);
    UpdateUser(&user05, "resU 50", sha256_09, salt_09, server::UserAccessLevel::kAdministratorUserAccessLevel);
    UpdateUser(&user09, "resU 90", sha256_01, salt_01, server::UserAccessLevel::kAdministratorUserAccessLevel);

    // Check table
    TestTableContent(users, sizeof(users) / sizeof(UserCache*));

    // Check table size
    TestTableSize(10);
}
BOOST_AUTO_TEST_CASE(test_partial_update_user_2)
{
    static const UserCache* users[] = {
        &user01, &user02, &user03, &user04, &user05,

        &user06, &user07, &user08, &user09, &user10,
    };

    // Update 5 users
    UpdateUser(&user03, "User 03", sha256_03, salt_03, server::UserAccessLevel::kRestrictedUserAccessLevel);
    UpdateUser(&user06, "User 06", sha256_06, salt_06, server::UserAccessLevel::kNormalUserAccessLevel);
    UpdateUser(&user07, "User 07", sha256_07, salt_07, server::UserAccessLevel::kNormalUserAccessLevel);
    UpdateUser(&user08, "User 08", sha256_08, salt_08, server::UserAccessLevel::kRestrictedUserAccessLevel);
    UpdateUser(&user10, "User 10", sha256_10, salt_10, server::UserAccessLevel::kAdministratorUserAccessLevel);

    // Check table
    TestTableContent(users, sizeof(users) / sizeof(UserCache*));

    // Re-Update 5 Users
    UpdateUser(&user01, "User 01", sha256_01, salt_01, server::UserAccessLevel::kAdministratorUserAccessLevel);
    UpdateUser(&user02, "User 02", sha256_02, salt_02, server::UserAccessLevel::kRestrictedUserAccessLevel);
    UpdateUser(&user04, "User 04", sha256_04, salt_04, server::UserAccessLevel::kMaintainerUserAccessLevel);
    UpdateUser(&user05, "User 05", sha256_05, salt_05, server::UserAccessLevel::kNormalUserAccessLevel);
    UpdateUser(&user09, "User 09", sha256_09, salt_09, server::UserAccessLevel::kRestrictedUserAccessLevel);

    // Check table
    TestTableContent(users, sizeof(users) / sizeof(UserCache*));

    // Check table size
    TestTableSize(10);
}

//! Test update prop
BOOST_AUTO_TEST_CASE(test_update_user_prop)
{
    static const UserCache* users[] = {
        &user01, &user02, &user03, &user04, &user05,

        &user06, &user07, &user08, &user09, &user10,
    };

    UpdateUserName(&user01, "Updated-User 1");
    UpdateUserName(&user02, "Updated-User 2");
    UpdateUserName(&user05, "This is just a long user name to check if very very long text is working correctly");
    UpdateUserName(&user02, "User 2 is a very good name");

    UpdateUserAccessLevel(&user04, server::UserAccessLevel::kRestrictedUserAccessLevel);
    UpdateUserAccessLevel(&user06, server::UserAccessLevel::kMaintainerUserAccessLevel);
    UpdateUserAccessLevel(&user04, server::UserAccessLevel::kMaintainerUserAccessLevel);
    UpdateUserAccessLevel(&user06, server::UserAccessLevel::kRestrictedUserAccessLevel);

    UpdateUserHash(&user10, sha256_04);
    UpdateUserHash(&user07, sha256_06);
    UpdateUserHash(&user09, sha256_08);
    UpdateUserHash(&user10, sha256_01);

    UpdateUserSalt(&user09, salt_08);
    UpdateUserSalt(&user07, salt_04);
    UpdateUserSalt(&user05, salt_03);
    UpdateUserSalt(&user02, salt_01);

    // Check table
    TestTableContent(users, sizeof(users) / sizeof(UserCache*));

    // Check table size
    TestTableSize(10);
}

//! Test update 2
BOOST_AUTO_TEST_CASE(test_update_user)
{
    static const UserCache* users[] = {
        &user01, &user02, &user03, &user04, &user05,

        &user06, &user07, &user08, &user09, &user10,
    };

    // Update all users
    UpdateUser(&user01, "User 01", sha256_02, salt_04, server::UserAccessLevel::kAdministratorUserAccessLevel);
    UpdateUser(&user02, "User 02", sha256_02, salt_04, server::UserAccessLevel::kAdministratorUserAccessLevel);
    UpdateUser(&user03, "User 03", sha256_02, salt_04, server::UserAccessLevel::kAdministratorUserAccessLevel);
    UpdateUser(&user04, "User 04", sha256_02, salt_04, server::UserAccessLevel::kAdministratorUserAccessLevel);
    UpdateUser(&user05, "User 05", sha256_02, salt_04, server::UserAccessLevel::kAdministratorUserAccessLevel);
    UpdateUser(&user06, "User 06", sha256_01, salt_03, server::UserAccessLevel::kAdministratorUserAccessLevel);
    UpdateUser(&user07, "User 07", sha256_01, salt_03, server::UserAccessLevel::kAdministratorUserAccessLevel);
    UpdateUser(&user08, "User 08", sha256_01, salt_03, server::UserAccessLevel::kAdministratorUserAccessLevel);
    UpdateUser(&user09, "User 09", sha256_01, salt_03, server::UserAccessLevel::kAdministratorUserAccessLevel);
    UpdateUser(&user10, "User 10", sha256_01, salt_03, server::UserAccessLevel::kAdministratorUserAccessLevel);

    // Check table
    TestTableContent(users, sizeof(users) / sizeof(UserCache*));

    // Check table size
    TestTableSize(10);
}

//! Test remove
BOOST_AUTO_TEST_CASE(test_partial_remove_user_1)
{
    static const UserCache* users[] = {
        &user03, &user06, &user07, &user08, &user10,
    };

    // Remove 5 users
    BOOST_CHECK_MESSAGE(database->RemoveUser(4) == true, "Remove user  4");
    BOOST_CHECK_MESSAGE(database->RemoveUser(2) == true, "Remove user  2");
    BOOST_CHECK_MESSAGE(database->RemoveUser(1) == true, "Remove user  1");
    BOOST_CHECK_MESSAGE(database->RemoveUser(5) == true, "Remove user  5");
    BOOST_CHECK_MESSAGE(database->RemoveUser(9) == true, "Remove user  9");

    // Remove 2 users twice
    BOOST_CHECK_MESSAGE(database->RemoveUser(2) == true, "Remove user  2 twice");
    BOOST_CHECK_MESSAGE(database->RemoveUser(4) == true, "Remove user  4 twice");

    // Check table
    TestTableContent(users, sizeof(users) / sizeof(UserCache*));

    // Check table size
    TestTableSize(5);
}
BOOST_AUTO_TEST_CASE(test_partial_remove_user_2)
{
    // Remove 5 users
    BOOST_CHECK_MESSAGE(database->RemoveUser(3) == true, "Remove user  3");
    BOOST_CHECK_MESSAGE(database->RemoveUser(6) == true, "Remove user  6");
    BOOST_CHECK_MESSAGE(database->RemoveUser(7) == true, "Remove user  7");
    BOOST_CHECK_MESSAGE(database->RemoveUser(8) == true, "Remove user  8");
    BOOST_CHECK_MESSAGE(database->RemoveUser(10) == true, "Remove user 10");

    // Check table
    TestTableContent(nullptr, 0);

    // Remove 3 users twice
    BOOST_CHECK_MESSAGE(database->RemoveUser(7) == true, "Remove user  7 twice");
    BOOST_CHECK_MESSAGE(database->RemoveUser(6) == true, "Remove user  6 twice");
    BOOST_CHECK_MESSAGE(database->RemoveUser(3) == true, "Remove user  3 twice");

    // Check table
    TestTableContent(nullptr, 0);

    // Check table size
    TestTableSize(0);
}

//! Big scale test
BOOST_AUTO_TEST_CASE(test_big_scale)
{
    static const size_t testSize = 2000;

    UserCache userList[testSize];
    UserCache* users[testSize];

    static uint8_t* sha256List[] = {
        sha256_01, sha256_02, sha256_03, sha256_04, sha256_05, sha256_06, sha256_07, sha256_08, sha256_09, sha256_10,
    };

    static uint8_t* saltList[] = {
        salt_01, salt_02, salt_03, salt_04, salt_05, salt_06, salt_07, salt_08, salt_09, salt_10,
    };

    static server::UserAccessLevel accessLevelList[] = {
        server::UserAccessLevel::kRestrictedUserAccessLevel,
        server::UserAccessLevel::kNormalUserAccessLevel,
        server::UserAccessLevel::kMaintainerUserAccessLevel,
        server::UserAccessLevel::kAdministratorUserAccessLevel,
    };

    //! Fill database
    for (size_t index = 0; index < testSize; index++)
    {
        UserCache& userCache = userList[index];
        userCache.id = database->ReserveUser();
        userCache.name = GenerateString(index % 256, index * 10);
        userCache.hash = sha256List[(index * 2) % 10];
        userCache.salt = saltList[(index * 3) % 10];
        userCache.accessLevel = accessLevelList[(index * 4) % 4];

        Ref<server::User> user =
            server::User::Create(userCache.name, userCache.id, userCache.hash, userCache.salt, userCache.accessLevel);
        database->UpdateUser(user);

        users[index] = &userCache;
    }

    //! Verify database

    // Check table
    TestTableContent((const UserCache**)users, testSize);

    // Check table size
    TestTableSize(testSize);
}