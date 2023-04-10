#include "SQLiteDatabase.hpp"

namespace server
{
    bool SQLiteDatabase::LoadScriptSources(
        const boost::function<void(identifier_t id, const std::string& type, const std::string& name,
                                   const std::string_view& config, const std::string_view& content)>& callback)
    {
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(select id, language, name, config, content from scriptsources)", -1,
                               &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failed to prepare sql load script sources statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        while (sqlite3_step(statement) == SQLITE_ROW)
        {
            identifier_t id = sqlite3_column_int64(statement, 0);

            //! Language field
            if (sqlite3_column_type(statement, 1) == SQLITE_NULL)
            {
                LOG_ERROR("Invalid type field of entity {0}", id);
                continue;
            }

            const unsigned char* language = sqlite3_column_text(statement, 1);
            size_t languageSize = sqlite3_column_bytes(statement, 1);

            //! Name field
            if (sqlite3_column_type(statement, 2) == SQLITE_NULL)
            {
                LOG_ERROR("Invalid name field of entity {0}", id);
                continue;
            }

            const unsigned char* name = sqlite3_column_text(statement, 2);
            size_t nameSize = sqlite3_column_bytes(statement, 2);
            if (name == nullptr)
            {
                name = (const uint8_t*)"";
                languageSize = 0;
            }

            //! Config field
            const unsigned char* config = sqlite3_column_text(statement, 3);
            size_t configSize = sqlite3_column_bytes(statement, 3);
            if (config == nullptr)
            {
                config = (const uint8_t*)"";
                configSize = 0;
            }

            //! Content field
            const void* content = sqlite3_column_text(statement, 4);
            size_t contentSize = sqlite3_column_bytes(statement, 4);
            if (content == nullptr)
            {
                content = (const uint8_t*)"";
                contentSize = 0;
            }

            callback(id, std::string((const char*)language, languageSize), std::string((const char*)name, nameSize),
                     std::string_view((const char*)config, configSize),
                     std::string_view((const char*)content, contentSize));
        }

        sqlite3_finalize(statement);
        return true;
    }

    identifier_t SQLiteDatabase::ReserveScriptSource(const std::string& language)
    {
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(
                connection,
                R"(insert into scriptsources values((select ifnull((select id+1 from scriptsources where (id+1) not in (select id from scriptsources) order by id asc limit 1), 1)), ?, "no name", null, null))",
                -1, &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failed to prepare sql reserve script source statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        if (sqlite3_bind_text(statement, 1, language.data(), language.size(), nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failed to bind sql parameters.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failed to execute sql reserve script source statement\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        identifier_t scriptSourceId = sqlite3_last_insert_rowid(connection);

        sqlite3_finalize(statement);
        return scriptSourceId;
    }

    bool SQLiteDatabase::UpdateScriptSource(identifier_t id, const std::string& name, const std::string_view& config)
    {
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update scriptsources set name = ?, config = ? where id = ?)", -1,
                               &statement, nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failed to prepare sql update script source statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_bind_text(statement, 1, name.data(), name.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_text(statement, 2, config.data(), config.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_int64(statement, 3, id) != SQLITE_OK)
        {
            LOG_ERROR("Failed to bind sql parameters.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failed to execute sql update script source statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);
        return true;
    }

    bool SQLiteDatabase::UpdateScriptSourceContent(identifier_t id, const std::string_view& newValue)
    {
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(update scriptsources set content = ? where id = ?)", -1, &statement,
                               nullptr) != SQLITE_OK)
        {
            LOG_ERROR("Failed to prepare sql update script source content statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_bind_blob(statement, 1, newValue.data(), newValue.size(), nullptr) != SQLITE_OK ||
            sqlite3_bind_int64(statement, 2, id) != SQLITE_OK)
        {
            LOG_ERROR("Failed to bind sql parameters.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            LOG_ERROR("Failed to execute sql update script source content statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);

        return true;
    }

    bool SQLiteDatabase::RemoveScriptSource(identifier_t id)
    {
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(delete from scriptsources where id = ?)", -1, &statement, nullptr) !=
            SQLITE_OK)
        {
            LOG_ERROR("Failed to prepare sql remove script source statement.\n{0}", sqlite3_errmsg(connection));
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
            LOG_ERROR("Failed to execute sql remove script source statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return false;
        }

        sqlite3_finalize(statement);
        return true;
    }

    size_t SQLiteDatabase::GetScriptSourceCount()
    {
        sqlite3_stmt* statement;

        if (sqlite3_prepare_v2(connection, R"(select count(*) from scriptsources)", -1, &statement, nullptr) !=
            SQLITE_OK)
        {
            LOG_ERROR("Failed to prepare sql count script sources statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        if (sqlite3_step(statement) != SQLITE_ROW)
        {
            LOG_ERROR("Failed to execute sql count script sources statement.\n{0}", sqlite3_errmsg(connection));
            sqlite3_finalize(statement);
            return 0;
        }

        size_t count = sqlite3_column_int64(statement, 0);

        sqlite3_finalize(statement);
        return count;
    }
}