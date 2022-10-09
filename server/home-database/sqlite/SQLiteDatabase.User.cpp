#include "SQLiteDatabase.hpp"
#include <cppcodec/base64_rfc4648.hpp>

namespace server
{
    bool SQLiteDatabase::LoadUsers(
        const boost::function<void(identifier_t id, const std::string& name, uint8_t hash[SHA256_SIZE],
                                   uint8_t salt[SALT_SIZE], const std::string& accessLevel)>& callback)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(select id, name, hash, salt, accesslevel, data from users)", 57,
                               &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        while (sqlite3_step(statement) == SQLITE_ROW)
        {
            identifier_t id = sqlite3_column_int64(statement, 0);

            // Check values
            if (sqlite3_column_type(statement, 1) == SQLITE_NULL || sqlite3_column_type(statement, 2) == SQLITE_NULL ||
                sqlite3_column_type(statement, 3) == SQLITE_NULL || sqlite3_column_type(statement, 4) == SQLITE_NULL ||
                sqlite3_column_type(statement, 5) == SQLITE_NULL)
            {
                LOG_ERROR("Failing to load invalid user {0}", id);
                continue;
            }

            const unsigned char* name = sqlite3_column_text(statement, 1);
            size_t nameSize = sqlite3_column_bytes(statement, 1);

            const unsigned char* hash = sqlite3_column_text(statement, 2);
            size_t hashSize = sqlite3_column_bytes(statement, 2);

            const unsigned char* salt = sqlite3_column_text(statement, 3);
            size_t saltSize = sqlite3_column_bytes(statement, 3);

            const unsigned char* accessLevel = sqlite3_column_text(statement, 4);
            size_t accessLevelSize = sqlite3_column_bytes(statement, 4);

            const unsigned char* data = sqlite3_column_text(statement, 5);
            size_t dataSize = sqlite3_column_bytes(statement, 5);

            // Check values
            if (name == nullptr || data == nullptr)
            {
                LOG_ERROR("Failing to load invalid user properties {0}", id);
                continue;
            }

            // Decode hash
            std::vector<uint8_t> hashBuffer = cppcodec::base64_rfc4648::decode((const char*)hash, hashSize);
            if (hashBuffer.size() != SHA256_SIZE)
            {
                LOG_ERROR("Failing to load invalid user hash {0}", id);
                continue;
            }

            // Decode salt
            std::vector<uint8_t> saltBuffer = cppcodec::base64_rfc4648::decode((const char*)salt, saltSize);
            if (saltBuffer.size() != SALT_SIZE)
            {
                LOG_ERROR("Failing to load invalid user salt {0}", id);
                continue;
            }

            callback(id, std::string((const char*)name, nameSize), hashBuffer.data(), saltBuffer.data(),
                     std::string((const char*)accessLevel, accessLevelSize));
        }

        sqlite3_finalize(statement);
        return true;
    }

    identifier_t SQLiteDatabase::ReserveUser()
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(
                connection,
                R"(insert into users values)"
                R"(((select ifnull((select id+1 from users where (id+1) not in (select id from users) order by id asc limit 1), 1)),)"
                R"(null, "", "", "restricted", "{}"))",
                -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failing to insert user into 'users' table.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        identifier_t id = sqlite3_last_insert_rowid(connection);

        sqlite3_finalize(statement);
        return id;
    }

    bool SQLiteDatabase::UpdateUser(identifier_t id, const std::string& name, uint8_t hash[SHA256_SIZE],
                                    uint8_t salt[SALT_SIZE], const std::string& accessLevel)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(replace into users values (?, ?, ?, ?, ?, "{}"))", -1, &statement,
                               nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        std::string hashBase64 = cppcodec::base64_rfc4648::encode(hash, SHA256_SIZE);
        std::string saltBase64 = cppcodec::base64_rfc4648::encode(salt, SALT_SIZE);

        if (sqlite3_bind_int64(statement, 1, id) != SQLITE_OK ||
            sqlite3_bind_text(statement, 2, name.data(), name.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_text(statement, 3, hashBase64.data(), hashBase64.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_text(statement, 4, saltBase64.data(), saltBase64.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_text(statement, 5, accessLevel.data(), accessLevel.size(), nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failing to insert user into 'users' table.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);
        return true;
    }

    bool SQLiteDatabase::UpdateUserPropName(identifier_t id, const std::string& newValue)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update users set name = ? where id = ?)", -1, &statement, nullptr) !=
            SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_bind_text(statement, 1, newValue.data(), newValue.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_int64(statement, 2, id) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failing to update user name.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);

        return true;
    }
    bool SQLiteDatabase::UpdateUserPropAccessLevel(identifier_t id, const std::string& newValue)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update users set accesslevel = ? where id = ?)", -1, &statement,
                               nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_bind_text(statement, 1, newValue.data(), newValue.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_int64(statement, 2, id) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failing to update user accesslevel.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);

        return true;
    }
    bool SQLiteDatabase::UpdateUserPropHash(identifier_t id, uint8_t newValue[SHA256_SIZE])
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update users set hash = ? where id = ?)", -1, &statement, nullptr) !=
            SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        std::string hash = cppcodec::base64_rfc4648::encode(newValue, SHA256_SIZE);

        if (sqlite3_bind_text(statement, 1, hash.data(), hash.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_int64(statement, 2, id) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failing to update user hash.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);

        return true;
    }
    bool SQLiteDatabase::UpdateUserPropSalt(identifier_t id, uint8_t newValue[SALT_SIZE])
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update users set salt = ? where id = ?)", -1, &statement, nullptr) !=
            SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        std::string salt = cppcodec::base64_rfc4648::encode(newValue, SALT_SIZE);

        if (sqlite3_bind_text(statement, 1, salt.data(), salt.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_int64(statement, 2, id) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failing to update user salt.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);

        return true;
    }

    bool SQLiteDatabase::RemoveUser(identifier_t id)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(delete from users where id = ?)", -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_bind_int64(statement, 1, id) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failing to delete user from 'users' table.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);
        return true;
    }

    size_t SQLiteDatabase::GetUserCount()
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(select count(*) from users)", -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        if (sqlite3_step(statement) != SQLITE_ROW)
        {
            LOG_ERROR("Failing to count users.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        size_t count = sqlite3_column_int64(statement, 0);

        sqlite3_finalize(statement);
        return count;
    }
}