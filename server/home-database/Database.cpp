#include "Database.hpp"
#include "sqlite/SQLiteDatabase.hpp"

namespace server
{
    std::string StringifyDatabaseType(DatabaseType type)
    {
        switch (type)
        {
        case DatabaseType::kSQLiteDatabaseType:
            return "sqlite";
        default:
            return "unknown";
        }
    }
    DatabaseType ParseDatabaseType(const std::string& type)
    {
        switch (crc32(type.data(), type.size()))
        {
        case CRC32("sqlite"):
            return DatabaseType::kSQLiteDatabaseType;
        default:
            return DatabaseType::kUnknownDatabaseType;
        }
    }

    WeakRef<Database> instanceDatabase;

    Database::Database()
    {
    }
    Database::~Database()
    {
    }

    Ref<Database> Database::Create(DatabaseType type, const std::string& db, const std::string& username,
                                   const std::string& password)
    {
        if (!instanceDatabase.expired())
            return Ref<Database>(instanceDatabase);

        Ref<Database> database;

        // Create database
        switch (type)
        {
        case DatabaseType::kSQLiteDatabaseType:
            database = SQLiteDatabase::Create(db);
            break;
        default:
            database = nullptr;
            break;
        }

        // Assign singleton reference
        if (database != nullptr)
            instanceDatabase = database;

        return database;
    }
    Ref<Database> Database::GetInstance()
    {
        return Ref<Database>(instanceDatabase);
    }
}