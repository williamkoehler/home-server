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
            db = boost::filesystem::absolute(db, config::GetStateDirectory()).string();

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
                        R"((id integer not null primary key, name text, scriptsourceid integer not null, roomid integer, data text not null, foreign key (roomid) references rooms (id)))",
                        nullptr, nullptr, &err) != SQLITE_OK)
                {
                    LOG_ERROR("Failing to create 'devices' table.\n{0}", err);
                    return nullptr;
                }

                // Service Table
                if (sqlite3_exec(
                        database->connection,
                        R"(create table if not exists services)"
                        R"((id integer not null primary key, name text, scriptsourceid integer not null, data text not null))",
                        nullptr, nullptr, &err) != SQLITE_OK)
                {
                    LOG_ERROR("Failing to create 'services' table.\n{0}", err);
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
}