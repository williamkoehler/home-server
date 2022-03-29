#include "SQLiteDatabase.hpp"
#include <cppcodec/base64_rfc4648.hpp>

namespace server
{
    SQLiteDatabase::SQLiteDatabase()
    {
    }
    SQLiteDatabase::~SQLiteDatabase()
    {
        if (connection != nullptr)
        {
            sqlite3_close(connection);
            connection = nullptr;
        }
    }
    Ref<SQLiteDatabase> SQLiteDatabase::Create(const std::string& db)
    {
        Ref<SQLiteDatabase> database = boost::make_shared<SQLiteDatabase>();
        if (database != nullptr)
        {
            // Open / Create database
            if (sqlite3_open_v2(db.c_str(), &database->connection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                nullptr) != SQLITE_OK)
            {
                LOG_ERROR("Failing to open/create sqlite database.\n{0}", sqlite3_errcode(database->connection));
                return nullptr;
            }

            // Create necessary tables
            char* err = nullptr;

            // Scripting
            {
                // Script Source Table
                if (sqlite3_exec(
                        database->connection,
                        R"(create table if not exists scriptsources)"
                        R"((id integer not null primary key, type text not null, name text, language text not null, content blob not null))",
                        nullptr, nullptr, &err) != SQLITE_OK)
                {
                    LOG_ERROR("Failing to create 'scriptsources' table.\n{0}", err);
                    return nullptr;
                }
            }

            // Home
            {
                // Room table
                if (sqlite3_exec(database->connection,
                                 R"(create table if not exists rooms)"
                                 R"((id integer not null primary key, type text not null, name text))",
                                 nullptr, nullptr, &err) != SQLITE_OK)
                {
                    LOG_ERROR("Failing to create 'rooms' table.\n{0}", err);
                    return nullptr;
                }

                // Device Table
                if (sqlite3_exec(
                        database->connection,
                        R"(create table if not exists devices)"
                        R"((id integer not null primary key, name text, scriptsourceid integer not null, controllerid integer, roomid integer, data text not null, foreign key (controllerid) references devices (id), foreign key (roomid) references rooms (id)))",
                        nullptr, nullptr, &err) != SQLITE_OK)
                {
                    LOG_ERROR("Failing to create 'devices' table.\n{0}", err);
                    return nullptr;
                }
            }

            // User
            {
                // User Table
                if (sqlite3_exec(
                        database->connection,
                        R"(create table if not exists users)"
                        R"((id integer not null primary key, name text, hash text not null, salt text not null, accesslevel text not null, data text not null))",
                        nullptr, nullptr, &err) != SQLITE_OK)
                {
                    LOG_ERROR("Failing to create 'users' table.\n{0}", err);
                    return nullptr;
                }
            }
        }

        return database;
    }

    //! ScriptSource
    bool SQLiteDatabase::LoadScriptSources(
        const boost::function<void(identifier_t id, const std::string& type, const std::string& name,
                                   const std::string& usage, const std::string_view& content)>& callback)
    {
        // Lock main mutex
        boost::lock_guard lock(mutex);

        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(select id, type, name, language, content from scriptsources)", -1,
                               &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        while (sqlite3_step(statement) == SQLITE_ROW)
        {
            identifier_t id = sqlite3_column_int64(statement, 0);

            // Check for null
            if (sqlite3_column_type(statement, 1) == SQLITE_NULL || sqlite3_column_type(statement, 2) == SQLITE_NULL ||
                sqlite3_column_type(statement, 3) == SQLITE_NULL || sqlite3_column_type(statement, 4) == SQLITE_NULL)
            {
                LOG_ERROR("Failing to load invalid room {0}");
                continue;
            }

            const unsigned char* type = sqlite3_column_text(statement, 1);
            size_t typeSize = sqlite3_column_bytes(statement, 1);

            const unsigned char* name = sqlite3_column_text(statement, 2);
            size_t nameSize = sqlite3_column_bytes(statement, 2);

            const unsigned char* language = sqlite3_column_text(statement, 3);
            size_t languageSize = sqlite3_column_bytes(statement, 3);

            const void* content = sqlite3_column_text(statement, 4);
            size_t contentSize = sqlite3_column_bytes(statement, 4);

            // Check values
            if (type == nullptr || name == nullptr || language == nullptr || content == nullptr)
            {
                LOG_ERROR("Failing to load invalid script source properties {0}", id);
                continue;
            }

            callback(id, std::string((const char*)type, typeSize), std::string((const char*)name, nameSize),
                     std::string((const char*)language, languageSize), std::string_view((const char*)content, contentSize));
        }

        sqlite3_finalize(statement);
        return true;
    }

    identifier_t SQLiteDatabase::ReserveScriptSource()
    {
        // Lock main mutex
        boost::lock_guard lock(mutex);

        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(
                connection,
                R"(insert into scriptsources values)"
                R"(((select ifnull((select id+1 from scriptsources where (id+1) not in (select id from scriptsources) order by id asc limit 1), 1)),)"
                R"("unknown", null, "unknown", ""))",
                -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failing to insert user into 'scriptsources' table.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        identifier_t sourceID = sqlite3_last_insert_rowid(connection);

        sqlite3_finalize(statement);
        return sourceID;
    }

    bool SQLiteDatabase::UpdateScriptSource(identifier_t id, const std::string& type, const std::string& name,
                                            const std::string& usage, const std::string_view& content)
    {
        // Lock main mutex
        boost::lock_guard lock(mutex);

        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(replace into scriptsources values (?, ?, ?, ?, ?))", -1, &statement,
                               nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_bind_int64(statement, 1, id) != SQLITE_OK ||
            sqlite3_bind_text(statement, 2, type.data(), type.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_text(statement, 3, name.data(), name.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_text(statement, 4, usage.data(), usage.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_blob(statement, 5, content.data(), content.size(), nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failing to insert script source into 'scriptsources' table.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);
        return true;
    }
    bool SQLiteDatabase::UpdateScriptSourcePropName(identifier_t id, const std::string& value,
                                                    const std::string& newValue)
    {
        // Lock main mutex
        boost::lock_guard lock(mutex);

        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update scriptsources set name = ? where id = ?)", -1, &statement,
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
            LOG_ERROR("Failing to update script source name.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);

        return true;
    }
    bool SQLiteDatabase::UpdateScriptSourcePropContent(identifier_t id, const std::string_view& newValue)
    {
        // Lock main mutex
        boost::lock_guard lock(mutex);

        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update scriptsources set content = ? where id = ?)", -1, &statement,
                               nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_bind_blob(statement, 1, newValue.data(), newValue.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_int64(statement, 2, id) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failing to update script source data.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);

        return true;
    }

    bool SQLiteDatabase::RemoveScriptSource(identifier_t id)
    {
        // Lock main mutex
        boost::lock_guard lock(mutex);

        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(delete from scriptsources where id = ?)", -1, &statement, nullptr) !=
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
            LOG_ERROR("Failing to delete script source from 'scriptsources' table.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);
        return true;
    }

    size_t SQLiteDatabase::GetScriptSourceCount()
    {
        // Lock main mutex
        boost::lock_guard lock(mutex);

        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(select count(*) from scriptsources)", -1, &statement, nullptr) !=
            SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        if (sqlite3_step(statement) != SQLITE_ROW)
        {
            LOG_ERROR("Failing to count script sources.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        size_t count = sqlite3_column_int64(statement, 0);

        sqlite3_finalize(statement);
        return count;
    }

    //! Room
    bool SQLiteDatabase::LoadRooms(
        const boost::function<bool(identifier_t id, const std::string& type, const std::string& name)>& callback)
    {
        // Lock main mutex
        boost::lock_guard lock(mutex);

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
        // Lock main mutex
        boost::lock_guard lock(mutex);

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
        // Lock main mutex
        boost::lock_guard lock(mutex);

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
        // Lock main mutex
        boost::lock_guard lock(mutex);

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
        // Lock main mutex
        boost::lock_guard lock(mutex);

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
        // Lock main mutex
        boost::lock_guard lock(mutex);

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
        // Lock main mutex
        boost::lock_guard lock(mutex);

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

    //! Device
    bool SQLiteDatabase::LoadDevices(
        const boost::function<bool(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                                   identifier_t controllerID, identifier_t roomID, const std::string_view& data)>&
            callback)
    {
        // Lock main mutex
        boost::lock_guard lock(mutex);

        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection,
                               R"(select id, name, scriptsourceid, controllerid, roomid, data from devices)", -1,
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
            if (sqlite3_column_type(statement, 1) == SQLITE_NULL || sqlite3_column_type(statement, 5) == SQLITE_NULL)
            {
                LOG_ERROR("Failing to load invalid device {0}", id);
                continue;
            }

            const unsigned char* name = sqlite3_column_text(statement, 1);
            size_t nameSize = sqlite3_column_bytes(statement, 1);

            identifier_t backendID = sqlite3_column_int64(statement, 2);

            identifier_t controllerID = sqlite3_column_int64(statement, 3);

            identifier_t roomID = sqlite3_column_int64(statement, 4);

            const unsigned char* data = sqlite3_column_text(statement, 5);
            size_t dataSize = sqlite3_column_bytes(statement, 5);

            // Check values
            if (name == nullptr || data == nullptr)
            {
                LOG_ERROR("Failing to load invalid device properties {0}", id);
                continue;
            }

            callback(id, std::string((const char*)name, nameSize), backendID, controllerID, roomID,
                     std::string_view((const char*)data, dataSize));
        }

        sqlite3_finalize(statement);
        return true;
    }

    identifier_t SQLiteDatabase::ReserveDevice()
    {
        // Lock main mutex
        boost::lock_guard lock(mutex);

        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(
                connection,
                R"(insert into devices values)"
                R"(((select ifnull((select id+1 from devices where (id+1) not in (select id from devices) order by id asc limit 1), 1)),)"
                R"(null, 0, null, null, "{}"))",
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
                                      identifier_t controllerID, identifier_t roomID)
    {
        // Lock main mutex
        boost::lock_guard lock(mutex);

        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(replace into devices values (?, ?, ?, ?, ?, "{}"))", -1, &statement,
                               nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failing to prepare sql statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_bind_int64(statement, 1, id) != SQLITE_OK ||
            sqlite3_bind_text(statement, 2, name.data(), name.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_int64(statement, 3, scriptSourceID) != SQLITE_OK ||
            sqlite3_bind_int64(statement, 4, controllerID) != SQLITE_OK ||
            sqlite3_bind_int64(statement, 5, roomID) != SQLITE_OK)
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
        // Lock main mutex
        boost::lock_guard lock(mutex);

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
    }bool SQLiteDatabase::UpdateDevicePropScriptSource(identifier_t id, identifier_t newValue)
    {
        // Lock main mutex
        boost::lock_guard lock(mutex);

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
    bool SQLiteDatabase::UpdateDevicePropController(identifier_t id, identifier_t value, identifier_t newValue)
    {
        // Lock main mutex
        boost::lock_guard lock(mutex);

        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update devices set controllerid = ? where id = ?)", -1, &statement,
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
            LOG_ERROR("Failing to update device controller.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);

        return true;
    }
    bool SQLiteDatabase::UpdateDevicePropRoom(identifier_t id, identifier_t value, identifier_t newValue)
    {
        // Lock main mutex
        boost::lock_guard lock(mutex);

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
        // Lock main mutex
        boost::lock_guard lock(mutex);

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
        // Lock main mutex
        boost::lock_guard lock(mutex);

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

    //! User
    bool SQLiteDatabase::LoadUsers(
        const boost::function<void(identifier_t id, const std::string& name, uint8_t hash[SHA256_SIZE],
                                   uint8_t salt[SALT_SIZE], const std::string& accessLevel)>& callback)
    {
        // Lock main mutex
        boost::lock_guard lock(mutex);

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
        // Lock main mutex
        boost::lock_guard lock(mutex);

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
        // Lock main mutex
        boost::lock_guard lock(mutex);

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

    bool SQLiteDatabase::UpdateUserPropName(identifier_t id, const std::string& value, const std::string& newValue)
    {
        // Lock main mutex
        boost::lock_guard lock(mutex);

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
    bool SQLiteDatabase::UpdateUserPropAccessLevel(identifier_t id, const std::string& value,
                                                   const std::string& newValue)
    {
        // Lock main mutex
        boost::lock_guard lock(mutex);

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
        // Lock main mutex
        boost::lock_guard lock(mutex);

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
        // Lock main mutex
        boost::lock_guard lock(mutex);

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
        // Lock main mutex
        boost::lock_guard lock(mutex);

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
        // Lock main mutex
        boost::lock_guard lock(mutex);

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