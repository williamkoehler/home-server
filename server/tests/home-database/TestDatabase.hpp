#pragma once
#include "../common.hpp"
#include "home-database/Database.hpp"

struct ScriptSource
{
    std::string type;
    std::string name;
    std::string usage;
    std::string content;
};

void CheckScriptSources(Ref<server::Database> database,
                        robin_hood::unordered_map<identifier_t, ScriptSource> scriptSources)
{
    size_t count = 0;

    database->LoadScriptSources(
        [&](identifier_t id, const std::string& type, const std::string& name, const std::string& usage,
            const std::string_view& content) -> void
        {
            robin_hood::unordered_map<identifier_t, ScriptSource>::const_iterator it = scriptSources.find(id);
            if (it != scriptSources.end())
            {
                const ScriptSource& scriptSource = it->second;

                BOOST_CHECK_MESSAGE(scriptSource.type == type, "Script source type does not match.");
                BOOST_CHECK_MESSAGE(scriptSource.name == name, "Script source name does not match.");
                BOOST_CHECK_MESSAGE(scriptSource.usage == usage, "Script source usage does not match.");
                BOOST_CHECK_MESSAGE(scriptSource.content == content, "Script source content does not match.");
            }
            else
            {
                BOOST_WARN_MESSAGE(it != scriptSources.end(), "Script source does not exist.");
            }

            count++;
        });

    BOOST_CHECK_MESSAGE(count >= scriptSources.size(), "Too few script sources.");
    BOOST_CHECK_MESSAGE(count <= scriptSources.size(), "Too many script sources.");
}

struct Room
{
    std::string type;
    std::string name;
};

void CheckRooms(Ref<server::Database> database, robin_hood::unordered_map<identifier_t, Room> rooms)
{
    size_t count = 0;

    database->LoadRooms(
        [&](identifier_t id, const std::string& type, const std::string& name) -> bool
        {
            robin_hood::unordered_map<identifier_t, Room>::const_iterator it = rooms.find(id);
            if (it != rooms.end())
            {
                const Room& room = it->second;

                BOOST_CHECK_MESSAGE(room.type == type, "Room type does not match.");
                BOOST_CHECK_MESSAGE(room.name == name, "Room name does not match.");
            }
            else
            {
                BOOST_WARN_MESSAGE(it != rooms.end(), "Room does not exist.");
            }

            count++;

            return true;
        });

    BOOST_CHECK_MESSAGE(count >= rooms.size(), "Too few rooms.");
    BOOST_CHECK_MESSAGE(count <= rooms.size(), "Too many rooms.");
}

struct Device
{
    std::string name;
    identifier_t scriptSourceID;
    identifier_t controllerID;
    identifier_t roomID;
    std::string data;
};

void CheckDevices(Ref<server::Database> database, robin_hood::unordered_map<identifier_t, Device> devices)
{
    size_t count = 0;

    database->LoadDevices(
        [&](identifier_t id, const std::string& name, identifier_t scriptSourceID, identifier_t controllerID,
            identifier_t roomID, const std::string_view& data) -> bool
        {
            robin_hood::unordered_map<identifier_t, Device>::const_iterator it = devices.find(id);
            if (it != devices.end())
            {
                const Device& device = it->second;

                BOOST_CHECK_MESSAGE(device.name == name, "Device name does not match.");
                BOOST_CHECK_MESSAGE(device.scriptSourceID == scriptSourceID, "Device script source id does not match.");
                BOOST_CHECK_MESSAGE(device.controllerID == controllerID, "Device controller id does not match.");
                BOOST_CHECK_MESSAGE(device.roomID == roomID, "Device room id does not match.");
                BOOST_CHECK_MESSAGE(device.data == data, "Device data does not match.");
            }
            else
            {
                BOOST_WARN_MESSAGE(it != devices.end(), "Device does not exist.");
            }

            count++;

            return true;
        });

    BOOST_CHECK_MESSAGE(count >= devices.size(), "Too few devices.");
    BOOST_CHECK_MESSAGE(count <= devices.size(), "Too many devices.");
}

struct User
{
    std::string name;
    uint8_t hash[SHA256_SIZE];
    uint8_t salt[SALT_SIZE];
    std::string accessLevel;
};

void CheckUsers(Ref<server::Database> database, robin_hood::unordered_map<identifier_t, User> users)
{
    size_t count = 0;

    database->LoadUsers(
        [&](identifier_t id, const std::string& name, uint8_t hash[SHA256_SIZE], uint8_t salt[SALT_SIZE],
            const std::string& accessLevel) -> bool
        {
            robin_hood::unordered_map<identifier_t, User>::const_iterator it = users.find(id);
            if (it != users.end())
            {
                const User& user = it->second;

                BOOST_CHECK_MESSAGE(user.name == name, "User name does not match.");
                BOOST_CHECK_MESSAGE(memcmp(user.hash, hash, SHA256_SIZE) == 0, "User hash does not match.");
                BOOST_CHECK_MESSAGE(memcmp(user.salt, salt, SALT_SIZE) == 0, "User salt does not match.");
                BOOST_CHECK_MESSAGE(user.accessLevel == accessLevel, "User access level does not match.");
            }
            else
            {
                BOOST_WARN_MESSAGE(it != users.end(), "User does not exist.");
            }

            count++;

            return true;
        });

    BOOST_CHECK_MESSAGE(count >= users.size(), "Too few users.");
    BOOST_CHECK_MESSAGE(count <= users.size(), "Too many users.");
}