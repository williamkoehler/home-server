#include "SQLiteDatabase.hpp"

namespace server
{
    bool SQLiteDatabase::LoadDevices(
        const boost::function<bool(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                                   identifier_t roomID, const std::string_view& data)>& callback)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(select id, name, scriptsourceid, roomid, data from devices)", -1,
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
            if (sqlite3_column_type(statement, 1) == SQLITE_NULL || sqlite3_column_type(statement, 4) == SQLITE_NULL)
            {
                LOG_ERROR("Failing to load invalid device {0}", id);
                continue;
            }

            const unsigned char* name = sqlite3_column_text(statement, 1);
            size_t nameSize = sqlite3_column_bytes(statement, 1);

            identifier_t backendID = sqlite3_column_int64(statement, 2);

            identifier_t roomID = sqlite3_column_int64(statement, 3);

            const unsigned char* data = sqlite3_column_text(statement, 4);
            size_t dataSize = sqlite3_column_bytes(statement, 4);

            // Check values
            if (name == nullptr || data == nullptr)
            {
                LOG_ERROR("Failing to load invalid device properties {0}", id);
                continue;
            }

            callback(id, std::string((const char*)name, nameSize), backendID, roomID,
                     std::string_view((const char*)data, dataSize));
        }

        sqlite3_finalize(statement);
        return true;
    }

    identifier_t SQLiteDatabase::ReserveDevice()
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(
                connection,
                R"(insert into devices values)"
                R"(((select ifnull((select id+1 from devices where (id+1) not in (select id from devices) order by id asc limit 1), 1)),)"
                R"(null, 0, null, "{}"))",
                -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failing to insert device into 'devices' table.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        identifier_t id = sqlite3_last_insert_rowid(connection);

        sqlite3_finalize(statement);
        return id;
    }

    bool SQLiteDatabase::UpdateDevice(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                                      identifier_t roomID)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(replace into devices values (?, ?, ?, ?, "{}"))", -1, &statement,
                               nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_bind_int64(statement, 1, id) != SQLITE_OK ||
            sqlite3_bind_text(statement, 2, name.data(), name.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_int64(statement, 3, scriptSourceID) != SQLITE_OK ||
            sqlite3_bind_int64(statement, 4, roomID) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failing to update device.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);
        return true;
    }
    bool SQLiteDatabase::UpdateDevicePropName(identifier_t id, const std::string& value, const std::string& newValue)
    {
        (void)value; // Will later be stored to allow history

        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update devices set name = ? where id = ?)", -1, &statement, nullptr) !=
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
            LOG_ERROR("Failing to update device name.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);

        return true;
    }
    bool SQLiteDatabase::UpdateDevicePropScriptSource(identifier_t id, identifier_t newValue)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update devices set scriptsourceid = ? where id = ?)", -1, &statement,
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
            LOG_ERROR("Failing to update device script source.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);

        return true;
    }
    bool SQLiteDatabase::UpdateDevicePropRoom(identifier_t id, identifier_t value, identifier_t newValue)
    {
        (void)value; // Will later be stored to allow history

        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update devices set roomid = ? where id = ?)", -1, &statement, nullptr) !=
            SQLITE_OK)
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
            LOG_ERROR("Failing to update device room.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);

        return true;
    }

    bool SQLiteDatabase::RemoveDevice(identifier_t id)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(delete from devices where id = ?)", -1, &statement, nullptr) != SQLITE_OK)
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
            LOG_ERROR("Failing to remove device from 'devices' table.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);
        return true;
    }

    size_t SQLiteDatabase::GetDeviceCount()
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(select count(*) from devices)", -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        if (sqlite3_step(statement) != SQLITE_ROW)
        {
            LOG_ERROR("Failing to count devices.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        size_t count = sqlite3_column_int64(statement, 0);

        sqlite3_finalize(statement);
        return count;
    }
}