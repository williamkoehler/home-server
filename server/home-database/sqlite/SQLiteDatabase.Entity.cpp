#include "SQLiteDatabase.hpp"

namespace server
{
    bool SQLiteDatabase::LoadEntities(
        const boost::function<bool(identifier_t id, const std::string& type, const std::string& name,
                                   identifier_t scriptSourceID, const std::string_view& config,
                                   const std::string_view& state)>& callback)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, "select id, type, name, scriptsourceid, config, state from entities", -1,
                               &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failed to prepare sql load entities statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        while (sqlite3_step(statement) == SQLITE_ROW)
        {
            identifier_t id = sqlite3_column_int64(statement, 0);

            //! Type field
            if (sqlite3_column_type(statement, 1) == SQLITE_NULL)
            {
                LOG_ERROR("Invalid type field of entity {0}", id);
                continue;
            }

            const unsigned char* type = sqlite3_column_text(statement, 1);
            size_t typeSize = sqlite3_column_bytes(statement, 1);

            //! Name field
            if (sqlite3_column_type(statement, 2) == SQLITE_NULL)
            {
                LOG_ERROR("Invalid name field of entity {0}", id);
                continue;
            }

            const unsigned char* name = sqlite3_column_text(statement, 2);
            size_t nameSize = sqlite3_column_bytes(statement, 2);

            //! Script source id field
            identifier_t scriptSourceId = sqlite3_column_int64(statement, 3);

            //! Config field
            const unsigned char* config = sqlite3_column_text(statement, 4);
            size_t configSize = sqlite3_column_bytes(statement, 4);
            if (config == nullptr)
            {
                config = (const uint8_t*)"";
                configSize = 0;
            }

            //! State field
            const unsigned char* state = sqlite3_column_text(statement, 5);
            size_t stateSize = sqlite3_column_bytes(statement, 5);
            if (state == nullptr)
            {
                state = (const uint8_t*)"";
                stateSize = 0;
            }

            callback(id, std::string((const char*)type, typeSize), std::string((const char*)name, nameSize),
                     scriptSourceId, std::string_view((const char*)config, configSize),
                     std::string_view((const char*)state, stateSize));
        }

        sqlite3_finalize(statement);
        return true;
    }

    identifier_t SQLiteDatabase::ReserveEntity(const std::string& type)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(
                connection,
                R"(insert into entities values((select ifnull((select (id+1) from entities where (id+1) not in (select id from entities) order by id asc limit 1), 1)), ?, "no name", 0, null, null))",
                -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failed to prepare sql reserve entity statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        if (sqlite3_bind_text(statement, 1, type.data(), type.size(), nullptr) != SQLITE_OK) // type
        {
            LOG_ERROR("Failed to bind sql parameters.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failed to execute sql reserve entity statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        identifier_t entityId = sqlite3_last_insert_rowid(connection);

        sqlite3_finalize(statement);
        return entityId;
    }

    bool SQLiteDatabase::UpdateEntity(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                                      const std::string_view& config)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, "update entities set name = ?, scriptsourceid = ?, config = ? where id = ?",
                               -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failed to prepare sql update entity statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_bind_text(statement, 1, name.data(), name.size(), nullptr) != SQLITE_OK ||     // name
            sqlite3_bind_int64(statement, 2, scriptSourceID) != SQLITE_OK ||                       // scriptsourceid
            sqlite3_bind_text(statement, 3, config.data(), config.size(), nullptr) != SQLITE_OK || // config
            sqlite3_bind_int64(statement, 4, id) != SQLITE_OK)                                     // id
        {
            LOG_ERROR("Failed to bind sql parameters.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failed to execute sql update entity statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);
        return true;
    }
    bool SQLiteDatabase::UpdateEntityState(identifier_t id, const std::string_view& state)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update entities set state = ? where id = ?)", -1, &statement, nullptr) !=
            SQLITE_OK)
        {
            LOG_ERROR("Failed to prepare sql update entity state statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_bind_text(statement, 1, state.data(), state.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_int64(statement, 2, id) != SQLITE_OK)
        {
            LOG_ERROR("Failed to bind sql parameters.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failed to execute sql update entity state statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);

        return true;
    }

    bool SQLiteDatabase::RemoveEntity(identifier_t id)
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, "delete from entities where id = ?", -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failed to prepare sql remove entity statement.\n{0}", sqlite3_errmsg(connection));
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
            LOG_ERROR("Failed to execute sql remove entity statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);
        return true;
    }

    size_t SQLiteDatabase::GetEntityCount()
    {
        // Insert into database
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, "select count(*) from entities", -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failed to prepare sql count entities statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        if (sqlite3_step(statement) != SQLITE_ROW)
        {
            LOG_ERROR("Failed to execute sql count entities statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        size_t count = sqlite3_column_int64(statement, 0);

        sqlite3_finalize(statement);
        return count;
    }
}