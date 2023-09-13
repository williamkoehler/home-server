#include "Testdatabase.hpp"
#include "../helper/random/String-random.hpp"

const boost::filesystem::path databaseFilepath = "test.sqlite3";
Ref<server::Database> database = nullptr;

robin_hood::unordered_map<identifier_t, ScriptSource> scriptSources;
robin_hood::unordered_map<identifier_t, Room> rooms;
robin_hood::unordered_map<identifier_t, Device> devices;
robin_hood::unordered_map<identifier_t, User> users;

#define TEST_SIZE (50)

BOOST_AUTO_TEST_CASE(test_database_initialize)
{
    // Delete any old database
    boost::filesystem::remove(databaseFilepath);

    // Create new database
    database = server::Database::Create(server::DatabaseType::kSQLiteDatabaseType, databaseFilepath.string());
    BOOST_CHECK_MESSAGE(database != nullptr, "Create database.");
}

BOOST_AUTO_TEST_CASE(test_database_reserve_object)
{
    for (size_t id = 1; id <= TEST_SIZE; id++)
    {
        // Test script source
        BOOST_CHECK_MESSAGE(database->ReserveScriptSource() == id, "Reserve script source.");
        scriptSources[id] = ScriptSource{
            "", // Type
            "", // Name
            "", // Usage
            "", // Content
        };

        // Test room
        BOOST_CHECK_MESSAGE(database->ReserveRoom() == id, "Reserve room.");
        rooms[id] = Room{
            "", // Type
            "", // Name
        };

        // Test device
        BOOST_CHECK_MESSAGE(database->ReserveDevice() == id, "Reserve device.");
        devices[id] = Device{
            "",   // Type
            0,    // Script Source ID
            0,    // Controller ID
            0,    // Room ID
            "{}", // Data
        };

        // Test user
        BOOST_CHECK_MESSAGE(database->ReserveUser() == id, "Reserve user.");
        users[id] = User{
            "", // Name
            {}, // Hash
            {}, // Salt
            "", // Access Level
        };
    }

    CheckScriptSources(database, robin_hood::unordered_map<identifier_t, ScriptSource>());
    CheckRooms(database, robin_hood::unordered_map<identifier_t, Room>());
    CheckDevices(database, robin_hood::unordered_map<identifier_t, Device>());
    CheckUsers(database, robin_hood::unordered_map<identifier_t, User>());
}

BOOST_AUTO_TEST_CASE(test_database_update_object)
{
    size_t seed = time(nullptr);

    for (size_t id = 1; id <= TEST_SIZE; id++)
    {
        // Test script source
        {
            ScriptSource& scriptSource = scriptSources.at(id);
            scriptSource.type = GenerateReadableRandomString(id % 20, seed + id + 11);
            scriptSource.name = GenerateReadableRandomString(id % 20, seed + id + 12);
            scriptSource.usage = GenerateReadableRandomString(id % 20, seed + id + 13);
            scriptSource.content = GenerateReadableRandomString(2000, seed + id + 14);

            BOOST_CHECK_MESSAGE(database->UpdateScriptSource(id, scriptSource.type, scriptSource.name,
                                                             scriptSource.usage, scriptSource.content) == true,
                                "Update script source.");
        }

        // Test room
        {
            Room& room = rooms.at(id);
            room.type = GenerateReadableRandomString(id % 20, seed + id + 21);
            room.name = GenerateReadableRandomString(id % 20, seed + id + 22);

            BOOST_CHECK_MESSAGE(database->UpdateRoom(id, room.type, room.name) == true, "Update room.");
        }

        // Test device
        {
            Device& device = devices.at(id);
            device.name = GenerateReadableRandomString(id % 20, seed + id + 31);
            device.scriptSourceID = seed + id + 32;
            device.controllerID = seed + id + 33;
            device.roomID = seed + id + 34;

            BOOST_CHECK_MESSAGE(database->UpdateDevice(id, device.name, device.scriptSourceID, device.controllerID,
                                                       device.roomID) == true,
                                "Update device.");
        }

        // Test user
        {
            User& user = users.at(id);
            user.name = GenerateReadableRandomString(id % 20, seed + id + 41);
            memcpy(user.hash, GenerateReadableRandomString(SHA256_SIZE, seed + id + 42).data(), SHA256_SIZE);
            memcpy(user.salt, GenerateReadableRandomString(SALT_SIZE, seed + id + 43).data(), SALT_SIZE);
            user.accessLevel = GenerateReadableRandomString(2000, seed + id + 44);

            BOOST_CHECK_MESSAGE(database->UpdateUser(id, user.name, user.hash, user.salt, user.accessLevel) == true,
                                "Update user.");
        }
    }

    CheckScriptSources(database, scriptSources);
    CheckRooms(database, rooms);
    CheckDevices(database, devices);
    CheckUsers(database, users);
}

BOOST_AUTO_TEST_CASE(test_database_reopen)
{
    // Close database
    database = nullptr;

    // Open database
    database = server::Database::Create(server::DatabaseType::kSQLiteDatabaseType, databaseFilepath.string());
    BOOST_CHECK_MESSAGE(database != nullptr, "Open database.");

    CheckScriptSources(database, scriptSources);
    CheckRooms(database, rooms);
    CheckDevices(database, devices);
    CheckUsers(database, users);
}

BOOST_AUTO_TEST_CASE(test_database_update_object_property)
{
    size_t seed = time(nullptr);

    for (size_t id = 1; id <= TEST_SIZE; id++)
    {
        // Test script source
        {
            ScriptSource& scriptSource = scriptSources.at(id);

            std::string oldName = scriptSource.name;
            scriptSource.name = GenerateReadableRandomString((seed * id) % 20, seed + id + 12);

            scriptSource.content = GenerateReadableRandomString(2000, seed + id + 14);

            BOOST_CHECK_MESSAGE(database->UpdateScriptSourcePropName(id, oldName, scriptSource.name) == true,
                                "Update script source name.");

            BOOST_CHECK_MESSAGE(database->UpdateScriptSourcePropContent(id, scriptSource.content) == true,
                                "Update script source content.");
        }

        // Test room
        {
            Room& room = rooms.at(id);

            std::string oldType = room.type;
            room.type = GenerateReadableRandomString((seed * id) % 20, seed + id + 21);

            std::string oldName = room.name;
            room.name = GenerateReadableRandomString((seed * id) % 20, seed + id + 22);

            BOOST_CHECK_MESSAGE(database->UpdateRoomPropType(id, oldType, room.type) == true, "Update room type.");

            BOOST_CHECK_MESSAGE(database->UpdateRoomPropName(id, oldName, room.name) == true, "Update room name.");
        }

        // Test device
        {
            Device& device = devices.at(id);

            std::string oldName = device.name;
            device.name = GenerateReadableRandomString((seed * id) % 20, seed + id + 31);

            device.scriptSourceID = seed + id + 32;

            identifier_t oldControllerID = device.controllerID;
            device.controllerID = seed + id + 33;

            identifier_t oldRoomID = device.roomID;
            device.roomID = seed + id + 34;

            BOOST_CHECK_MESSAGE(database->UpdateDevicePropName(id, oldName, device.name) == true,
                                "Update device name.");

            BOOST_CHECK_MESSAGE(database->UpdateDevicePropScriptSource(id, device.scriptSourceID) == true,
                                "Update device script source id.");

            BOOST_CHECK_MESSAGE(database->UpdateDevicePropController(id, oldControllerID, device.controllerID) == true,
                                "Update device controller id.");

            BOOST_CHECK_MESSAGE(database->UpdateDevicePropRoom(id, oldRoomID, device.roomID) == true,
                                "Update device room id.");
        }

        // Test user
        {
            User& user = users.at(id);

            user.name = GenerateReadableRandomString((seed * id) % 20, seed + id + 41);

            memcpy(user.hash, GenerateReadableRandomString(SHA256_SIZE, seed + id + 42).data(), SHA256_SIZE);

            memcpy(user.salt, GenerateReadableRandomString(SALT_SIZE, seed + id + 43).data(), SALT_SIZE);

            user.accessLevel = GenerateReadableRandomString((seed * id) % 20, seed + id + 44);

            BOOST_CHECK_MESSAGE(database->UpdateUserPropName(id, user.name) == true, "Update user name.");

            BOOST_CHECK_MESSAGE(database->UpdateUserPropHash(id, user.hash) == true, "Update user hash.");

            BOOST_CHECK_MESSAGE(database->UpdateUserPropSalt(id, user.salt) == true, "Update user salt.");

            BOOST_CHECK_MESSAGE(database->UpdateUserPropAccessLevel(id, user.accessLevel) == true,
                                "Update user access level.");
        }
    }

    CheckScriptSources(database, scriptSources);
    CheckRooms(database, rooms);
    CheckDevices(database, devices);
    CheckUsers(database, users);
}

BOOST_AUTO_TEST_CASE(test_database_remove_object)
{
    size_t seed = time(nullptr);

    for (size_t id = 1; id <= TEST_SIZE; id++)
    {
        // Test script source
        BOOST_CHECK_MESSAGE(database->RemoveScriptSource(id) == true, "Remove script source.");

        // Test room
        BOOST_CHECK_MESSAGE(database->RemoveRoom(id) == true, "Remove room.");

        // Test device
        BOOST_CHECK_MESSAGE(database->RemoveDevice(id) == true, "Remove device.");

        // Test user
        BOOST_CHECK_MESSAGE(database->RemoveUser(id) == true, "Remove user.");
    }

    CheckScriptSources(database, robin_hood::unordered_map<identifier_t, ScriptSource>());
    CheckRooms(database, robin_hood::unordered_map<identifier_t, Room>());
    CheckDevices(database, robin_hood::unordered_map<identifier_t, Device>());
    CheckUsers(database, robin_hood::unordered_map<identifier_t, User>());
}