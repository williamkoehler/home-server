#include "SQLiteDatabase.hpp"
#include <cppcodec/base64_rfc4648.hpp>

namespace server
{
    bool SQLiteDatabase::LoadUsers(
        const boost::function<void(identifier_t id, const std::string& name, uint8_t hash[SHA256_SIZE],
                                   uint8_t salt[SALT_SIZE], const std::string& accessLevel)>& callback)
    {
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(select id, name, hash, salt, accesslevel, config from users)", 57,
                               &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failed to prepare sql load users statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        bool error = false;

        while (sqlite3_step(statement) == SQLITE_ROW)
        {
            identifier_t id = sqlite3_column_int64(statement, 0);

            //! Name field
            if (sqlite3_column_type(statement, 1) != SQLITE_TEXT)
            {
                LOG_ERROR("Invalid name field of user {0}", id);
                continue;
            }

            const unsigned char* name = sqlite3_column_text(statement, 1);
            size_t nameSize = sqlite3_column_bytes(statement, 1);

            if (name == nullptr)
            {
                LOG_ERROR("Invalid name field text of user {0}", id);
                error = true;
                continue;
            }

            //! Hash field
            if (sqlite3_column_type(statement, 2) != SQLITE_TEXT)
            {
                LOG_ERROR("Invalid hash field of user {0}", id);
                error = true;
                continue;
            }

            const unsigned char* hash = sqlite3_column_text(statement, 2);
            size_t hashSize = sqlite3_column_bytes(statement, 2);

            if (hash == nullptr)
            {
                LOG_ERROR("Invalid hash field text of user {0}", id);
                error = true;
                continue;
            }

            std::vector<uint8_t> hashBuffer = cppcodec::base64_rfc4648::decode((const char*)hash, hashSize);
            if (hashBuffer.size() != SHA256_SIZE)
            {
                LOG_ERROR("Failed to parse base64 user hash of user {0}", id);
                error = true;
                continue;
            }

            //! Salt field
            if (sqlite3_column_type(statement, 3) != SQLITE_TEXT)
            {
                LOG_ERROR("Invalid salt field of user {0}", id);
                continue;
            }

            const unsigned char* salt = sqlite3_column_text(statement, 3);
            size_t saltSize = sqlite3_column_bytes(statement, 3);

            if (salt == nullptr)
            {
                LOG_ERROR("Invalid salt field text of user {0}", id);
                error = true;
                continue;
            }

            std::vector<uint8_t> saltBuffer = cppcodec::base64_rfc4648::decode((const char*)salt, saltSize);
            if (saltBuffer.size() != SALT_SIZE)
            {
                LOG_ERROR("Failed to parse base64 user salt of user {0}", id);
                error = true;
                continue;
            }

            //! Access level field
            if (sqlite3_column_type(statement, 4) != SQLITE_TEXT)
            {
                LOG_ERROR("Invalid access level field of user {0}", id);
                continue;
            }

            const unsigned char* accessLevel = sqlite3_column_text(statement, 4);
            size_t accessLevelSize = sqlite3_column_bytes(statement, 4);

            if (accessLevel == nullptr)
            {
                LOG_ERROR("Invalid access level field text of user {0}", id);
                error = true;
                continue;
            }

            //! Config field
            if (sqlite3_column_type(statement, 1) != SQLITE_TEXT)
            {
                LOG_ERROR("Invalid config field of user {0}", id);
                continue;
            }

            const unsigned char* config = sqlite3_column_text(statement, 5);
            size_t configSize = sqlite3_column_bytes(statement, 5);

            if (config == nullptr)
            {
                LOG_ERROR("Invalid config field text of user {0}", id);
                error = true;
                continue;
            }

            (void)config; // Will be used for further data
            (void)configSize;

            callback(id, std::string((const char*)name, nameSize), hashBuffer.data(), saltBuffer.data(),
                     std::string((const char*)accessLevel, accessLevelSize));
        }

        sqlite3_finalize(statement);
        return !error;
    }

    identifier_t SQLiteDatabase::ReserveUser(const std::string& name)
    {
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(
                connection,
                R"(insert into users values((select ifnull((select id+1 from users where (id+1) not in (select id from users) order by id asc limit 1), 1)), ?, "", "", "", null))",
                -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failed to prepare sql reserve user statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        if (sqlite3_bind_text(statement, 1, name.data(), name.size(), nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failed to bind sql parameters.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failed to execute sql reserve user statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        identifier_t id = sqlite3_last_insert_rowid(connection);

        sqlite3_finalize(statement);
        return id;
    }

    bool SQLiteDatabase::UpdateUserAccessLevel(identifier_t id, const std::string& newValue)
    {
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update users set accesslevel = ? where id = ?)", -1, &statement,
                               nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failed to prepare sql user user access level statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_bind_text(statement, 1, newValue.data(), newValue.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_int64(statement, 2, id) != SQLITE_OK)
        {
            LOG_ERROR("Failed to bind sql parameters.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failed to execute sql update user access level statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);

        return true;
    }
    bool SQLiteDatabase::UpdateUserHash(identifier_t id, uint8_t hash[SHA256_SIZE], uint8_t salt[SALT_SIZE])
    {
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update users set hash = ?, salt = ? where id = ?)", -1, &statement,
                               nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failed to prepare sql update user hash statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        std::string base64Hash = cppcodec::base64_rfc4648::encode(hash, SHA256_SIZE);
        std::string base64Salt = cppcodec::base64_rfc4648::encode(salt, SALT_SIZE);

        if (sqlite3_bind_text(statement, 1, base64Hash.data(), base64Hash.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_text(statement, 2, base64Salt.data(), base64Salt.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_int64(statement, 3, id) != SQLITE_OK)
        {
            LOG_ERROR("Failed to bind sql parameters.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failed to execute sql update user hash statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);

        return true;
    }

    bool SQLiteDatabase::RemoveUser(identifier_t id)
    {
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(delete from users where id = ?)", -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failed to prepare sql remove user statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_bind_int64(statement, 1, id) != SQLITE_OK)
        {
            LOG_ERROR("Failed to bind sql parameters.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failed to execute sql remove user statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);
        return true;
    }

    size_t SQLiteDatabase::GetUserCount()
    {
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(select count(*) from users)", -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failed to prepare sql count users statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        if (sqlite3_step(statement) != SQLITE_ROW)
        {
            LOG_ERROR("Failed to execute sql count users statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        size_t count = sqlite3_column_int64(statement, 0);

        sqlite3_finalize(statement);
        return count;
    }
}