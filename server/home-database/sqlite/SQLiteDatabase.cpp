#include "SQLiteDatabase.hpp"

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
    Ref<SQLiteDatabase> SQLiteDatabase::Create(std::string db)
    {
        Ref<SQLiteDatabase> database = boost::make_shared<SQLiteDatabase>();
        if (database != nullptr)
        {
            // Find path
            if (db.empty())
                db = "home.sqlite3";
            db = boost::filesystem::absolute(db, config::GetDataDirectory()).string();

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
                        R"((id integer not null primary key, language text not null, name text not null, config text, content blob))",
                        nullptr, nullptr, &err) != SQLITE_OK)
                {
                    LOG_ERROR("Failing to create 'scriptsources' table.\n{0}", err);
                    return nullptr;
                }
            }

            // Home
            {
                // Entity table
                if (sqlite3_exec(database->connection,
                                 R"(create table if not exists entities)"
                                 R"((id integer not null primary key, type text not null, name text not null, scriptsourceid integer, attributes text, state text))",
                                 nullptr, nullptr, &err) != SQLITE_OK)
                {
                    LOG_ERROR("Failing to create 'entities' table.\n{0}", err);
                    return nullptr;
                }
            }

            // User
            {
                // User Table
                if (sqlite3_exec(
                        database->connection,
                        R"(create table if not exists users)"
                        R"((id integer not null primary key, name text not null, hash text not null, salt text not null, accesslevel text not null, config text))",
                        nullptr, nullptr, &err) != SQLITE_OK)
                {
                    LOG_ERROR("Failing to create 'users' table.\n{0}", err);
                    return nullptr;
                }
            }
        }

        return database;
    }
}