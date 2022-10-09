#include "SQLiteDatabase.hpp"

namespace server
{
    bool SQLiteDatabase::LoadServices(
        const boost::function<bool(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                                   const std::string_view& data)>& callback)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(select id, name, scriptsourceid, data from services)", -1, &statement,
                               nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        while (sqlite3_step(statement) == SQLITE_ROW)
        {
            identifier_t id = sqlite3_column_int64(statement, 0);

            // Check values
            if (sqlite3_column_type(statement, 1) == SQLITE_NULL)
            {
                LOG_ERROR("Failing to load invalid service {0}", id);
                continue;
            }

            const unsigned char* name = sqlite3_column_text(statement, 1);
            size_t nameSize = sqlite3_column_bytes(statement, 1);

            identifier_t backendID = sqlite3_column_int64(statement, 2);

            const unsigned char* data = sqlite3_column_text(statement, 3);
            size_t dataSize = sqlite3_column_bytes(statement, 3);

            // Check values
            if (name == nullptr || data == nullptr)
            {
                LOG_ERROR("Failing to load invalid service properties {0}", id);
                continue;
            }

            callback(id, std::string((const char*)name, nameSize), backendID,
                     std::string_view((const char*)data, dataSize));
        }

        sqlite3_finalize(statement);
        return true;
    }

    identifier_t SQLiteDatabase::ReserveService()
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(
                connection,
                R"(insert into services values)"
                R"(((select ifnull((select id+1 from services where (id+1) not in (select id from services) order by id asc limit 1), 1)),)"
                R"(null, 0, "{}"))",
                -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failing to insert service into 'services' table.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        identifier_t id = sqlite3_last_insert_rowid(connection);

        sqlite3_finalize(statement);
        return id;
    }

    bool SQLiteDatabase::UpdateService(identifier_t id, const std::string& name, identifier_t scriptSourceID)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(replace into services values (?, ?, ?, "{}"))", -1, &statement,
                               nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_bind_int64(statement, 1, id) != SQLITE_OK ||
            sqlite3_bind_text(statement, 2, name.data(), name.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_int64(statement, 3, scriptSourceID) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failing to update service.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);
        return true;
    }
    bool SQLiteDatabase::UpdateServicePropName(identifier_t id, const std::string& value, const std::string& newValue)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update services set name = ? where id = ?)", -1, &statement, nullptr) !=
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
            LOG_ERROR("Failing to update service name.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);

        return true;
    }
    bool SQLiteDatabase::UpdateServicePropScriptSource(identifier_t id, identifier_t newValue)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update services set scriptsourceid = ? where id = ?)", -1, &statement,
                               nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_bind_int64(statement, 1, newValue) != SQLITE_OK ||
            sqlite3_bind_int64(statement, 2, id) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failing to update service script source.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);

        return true;
    }

    bool SQLiteDatabase::RemoveService(identifier_t id)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(delete from services where id = ?)", -1, &statement, nullptr) !=
            SQLITE_OK)
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
            LOG_ERROR("Failing to remove service from 'services' table.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);
        return true;
    }

    size_t SQLiteDatabase::GetServiceCount()
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(select count(*) from services)", -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        if (sqlite3_step(statement) != SQLITE_ROW)
        {
            LOG_ERROR("Failing to count services.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        size_t count = sqlite3_column_int64(statement, 0);

        sqlite3_finalize(statement);
        return count;
    }
}