#include "SQLiteDatabase.hpp"

namespace server
{
    bool SQLiteDatabase::LoadScriptSources(
        const boost::function<void(identifier_t id, const std::string& type, const std::string& name,
                                   const std::string& usage, const std::string_view& content)>& callback)
    {
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
                LOG_ERROR("Failing to load invalid script source {0}", id);
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
                     std::string((const char*)language, languageSize),
                     std::string_view((const char*)content, contentSize));
        }

        sqlite3_finalize(statement);
        return true;
    }

    identifier_t SQLiteDatabase::ReserveScriptSource()
    {
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
        (void)value; // Will later be stored to allow history

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
}