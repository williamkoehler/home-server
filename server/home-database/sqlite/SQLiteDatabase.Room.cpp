#include "SQLiteDatabase.hpp"

namespace server
{
    bool SQLiteDatabase::LoadRooms(
        const boost::function<bool(identifier_t id, const std::string& type, const std::string& name)>& callback)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(select id, type, name from rooms)", -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        while (sqlite3_step(statement) == SQLITE_ROW)
        {
            identifier_t id = sqlite3_column_int64(statement, 0);

            // Check for null
            if (sqlite3_column_type(statement, 1) == SQLITE_NULL || sqlite3_column_type(statement, 2) == SQLITE_NULL)
            {
                LOG_ERROR("Failing to load invalid room {0}", id);
                continue;
            }

            const unsigned char* type = sqlite3_column_text(statement, 1);
            size_t typeSize = sqlite3_column_bytes(statement, 1);

            const unsigned char* name = sqlite3_column_text(statement, 2);
            size_t nameSize = sqlite3_column_bytes(statement, 2);

            // Check values
            if (type == nullptr || name == nullptr)
            {
                LOG_ERROR("Failing to load invalid room properties {0}", id);
                continue;
            }

            callback(id, std::string((const char*)type, typeSize), std::string((const char*)name, nameSize));
        }

        sqlite3_finalize(statement);
        return true;
    }

    identifier_t SQLiteDatabase::ReserveRoom()
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(
                connection,
                R"(insert into rooms values)"
                R"(((select ifnull((select (id+1) from rooms where (id+1) not in (select id from rooms) order by id asc limit 1), 1)),)"
                R"("no type", null))",
                -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failing to delete room from 'rooms' table.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        identifier_t roomID = sqlite3_last_insert_rowid(connection);

        sqlite3_finalize(statement);
        return roomID;
    }

    bool SQLiteDatabase::UpdateRoom(identifier_t id, const std::string& type, const std::string& name)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(replace into rooms values (?, ?, ?))", -1, &statement, nullptr) !=
            SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_bind_int64(statement, 1, id) != SQLITE_OK ||
            sqlite3_bind_text(statement, 2, type.data(), type.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_text(statement, 3, name.data(), name.size(), nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failing to delete room from 'rooms' table.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);
        return true;
    }
    bool SQLiteDatabase::UpdateRoomPropType(identifier_t id, const std::string& value, const std::string& newValue)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update rooms set type = ? where id = ?)", -1, &statement, nullptr) !=
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
            LOG_ERROR("Failing to update room type.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);

        return true;
    }
    bool SQLiteDatabase::UpdateRoomPropName(identifier_t id, const std::string& value, const std::string& newValue)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update rooms set name = ? where id = ?)", -1, &statement, nullptr) !=
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
            LOG_ERROR("Failing to update room name.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);

        return true;
    }

    bool SQLiteDatabase::RemoveRoom(identifier_t roomID)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(delete from rooms where id = ?)", -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_bind_int64(statement, 1, roomID) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failing to delete room from 'rooms' table.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);
        return true;
    }

    size_t SQLiteDatabase::GetRoomCount()
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(select count(*) from rooms)", -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        if (sqlite3_step(statement) != SQLITE_ROW)
        {
            LOG_ERROR("Failing to count rooms.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        size_t count = sqlite3_column_int64(statement, 0);

        sqlite3_finalize(statement);
        return count;
    }
}