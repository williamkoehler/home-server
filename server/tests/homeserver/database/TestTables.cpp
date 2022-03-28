#include "TestDatabase.hpp"

struct Column
{
    std::string name;
    std::string type;
    uint8_t notNull;
    uint8_t primaryKey;
};
void TestTable(sqlite3* database, const char* table, const Column* columns, size_t columnCount)
{
    char* err = nullptr;
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(database, R"(select name, type, "notnull", pk from pragma_table_info(?))", 65, &statement,
                           nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(statement, 1, table, -1, 0);

        // Check every column
        size_t index = 0;
        for (; sqlite3_step(statement) == SQLITE_ROW && index < columnCount; index++)
        {
            std::string name = (const char*)sqlite3_column_text(statement, 0);
            std::string type = (const char*)sqlite3_column_text(statement, 1);
            int notNull = sqlite3_column_int(statement, 2);
            int primaryKey = sqlite3_column_int(statement, 3);

            const Column& column = columns[index];

            //! Check column name
            BOOST_CHECK_MESSAGE(name == column.name, "(Table '" << table << "') Column name is not correct: '" << name
                                                                << "' is not '" << column.name << "'");

            //! Check column type
            BOOST_CHECK_MESSAGE(type == column.type, "(Table '" << table << "') (Column '" << column.name
                                                                << "' Type is not correct: '" << type << "' is not '"
                                                                << column.type << "'");

            //! Check column not null flag
            if (column.notNull)
                BOOST_CHECK_MESSAGE(notNull == column.notNull,
                                    "(Table '" << table << "') (Column '" << column.name << "' NotNull flag is set");
            else
                BOOST_CHECK_MESSAGE(notNull == column.notNull, "(Table '" << table << "') (Column '" << column.name
                                                                          << "' NotNull flag is not set");

            //! Check column primary key flag
            if (column.primaryKey)
                BOOST_CHECK_MESSAGE(notNull == column.notNull, "(Table '" << table << "') (Column '" << column.name
                                                                          << "' Primary key flag is not set");
            else
                BOOST_CHECK_MESSAGE(notNull == column.notNull, "(Table '" << table << "') (Column '" << column.name
                                                                          << "' Primary key flag is set");
        }

        //! Compaire row count with column count
        BOOST_CHECK_MESSAGE(index <= columnCount, "(Table '" << table << "') Table has too many columns");
        BOOST_CHECK_MESSAGE(index >= columnCount, "(Table '" << table << "') Table has too few columns");

        // The check was successful
        sqlite3_finalize(statement);
    }
    else
        BOOST_TEST_ERROR("SQLite error: " << err);
}

//! Test script source table
BOOST_AUTO_TEST_CASE(test_database_script_source_table)
{
    BOOST_REQUIRE(database != nullptr);

    static const Column columns[] = {
        {"id", "integer", 1, 1},    {"name", "text", 0, 0}, {"usage", "text", 1, 0},
        {"language", "text", 1, 0}, {"data", "blob", 1, 0},
    };

    TestTable(database->GetConnection_TEST(), "scriptsources", columns, sizeof(columns) / sizeof(Column));
}

//! Test room table
BOOST_AUTO_TEST_CASE(test_database_room_table)
{
    BOOST_REQUIRE(database != nullptr);

    static const Column columns[] = {
        {"id", "integer", 1, 1},
        {"name", "text", 0, 0},
        {"type", "text", 1, 0},
    };

    TestTable(database->GetConnection_TEST(), "rooms", columns, sizeof(columns) / sizeof(Column));
}

//! Test device controller table
BOOST_AUTO_TEST_CASE(test_database_device_controllers_table)
{
    BOOST_REQUIRE(database != nullptr);

    static const Column columns[] = {
        {"id", "integer", 1, 1},     {"name", "text", 0, 0}, {"pluginid", "integer", 1, 0},
        {"roomid", "integer", 0, 0}, {"data", "text", 1, 0},
    };

    TestTable(database->GetConnection_TEST(), "devicecontrollers", columns, sizeof(columns) / sizeof(Column));
}

//! Test devices table
BOOST_AUTO_TEST_CASE(test_database_devices_table)
{
    BOOST_REQUIRE(database != nullptr);

    static const Column columns[] = {
        {"id", "integer", 1, 1},           {"name", "text", 0, 0},      {"pluginid", "integer", 1, 0},
        {"controllerid", "integer", 0, 0}, {"roomid", "integer", 0, 0}, {"data", "text", 1, 0},
    };

    TestTable(database->GetConnection_TEST(), "devices", columns, sizeof(columns) / sizeof(Column));
}

//! Test actions table
BOOST_AUTO_TEST_CASE(test_database_actions_table)
{
    BOOST_REQUIRE(database != nullptr);

    static const Column columns[] = {
        {"id", "integer", 1, 1},     {"name", "text", 0, 0}, {"sourceid", "integer", 1, 0},
        {"roomid", "integer", 0, 0}, {"data", "text", 1, 0},
    };

    TestTable(database->GetConnection_TEST(), "actions", columns, sizeof(columns) / sizeof(Column));
}

//! Test users table
BOOST_AUTO_TEST_CASE(test_database_users_table)
{
    BOOST_REQUIRE(database != nullptr);

    static const Column columns[] = {
        {"id", "integer", 1, 1}, {"name", "text", 0, 0},        {"hash", "text", 1, 0},
        {"salt", "text", 1, 0},  {"accesslevel", "text", 1, 0}, {"data", "text", 1, 0},
    };

    TestTable(database->GetConnection_TEST(), "users", columns, sizeof(columns) / sizeof(Column));
}