#pragma once
#include "common.hpp"

namespace server
{
    enum class DatabaseType
    {
        kUnknownDatabaseType,
        kEmptyDatabaseType,
        kSQLiteDatabaseType,
    };

    std::string StringifyDatabaseType(DatabaseType type);
    DatabaseType ParseDatabaseType(const std::string& type);

    class Database : public boost::enable_shared_from_this<Database>
    {
      public:
        Database();
        virtual ~Database();

        /// @brief Create database instance
        ///
        /// @param type Database type
        /// @param db Database location
        /// @param username Username (optional)
        /// @param password Password (optional)
        /// @return Database singleton
        static Ref<Database> Create(DatabaseType type, const std::string& db = "home.sqlite3",
                                    const std::string& username = "", const std::string& password = "");

        /// @brief Get database instance
        ///
        /// @return Database singleton
        static Ref<Database> GetInstance();

        //! ScriptSource

        /// @brief Load script sources from database
        ///
        /// @param callback Entry callback called once for every entry
        /// @return Successfulness
        virtual bool LoadScriptSources(
            const boost::function<void(identifier_t id, const std::string& language, const std::string& name,
                                       const std::string_view& config, const std::string_view& content)>& callback) = 0;

        /// @brief Reserve script source entry in database
        ///
        /// @param language Script source language
        /// @return identifier_t Script source id or zero if reservation failed
        virtual identifier_t ReserveScriptSource(const std::string& language) = 0;

        /// @brief Update script source
        ///
        /// @param id Script source id
        /// @param name Script source name
        /// @param content Additional config
        /// @return Successfulness
        virtual bool UpdateScriptSource(identifier_t id, const std::string& name, const std::string_view& config) = 0;

        /// @brief Update script source content
        ///
        /// @param id Script source id
        /// @param newValue New content
        /// @return Successfulness
        virtual bool UpdateScriptSourceContent(identifier_t id, const std::string_view& newValue) = 0;

        /// @brief Remove script source
        ///
        /// @param id Script source id
        /// @return Successfulness
        virtual bool RemoveScriptSource(identifier_t id) = 0;

        /// @brief Get script source count
        ///
        /// @return size_t Script source count
        virtual size_t GetScriptSourceCount() = 0;

        //! Entity

        /// @brief Load entities from database
        ///
        /// @param callback Entry callback
        /// @return Successfulness
        virtual bool LoadEntities(
            const boost::function<bool(identifier_t id, const std::string& entityType, const std::string& name,
                                       identifier_t scriptSourceID, const std::string_view& attributes,
                                       const std::string_view& state)>& callback) = 0;

        /// @brief Reserve entity entry in database
        ///
        /// @param type Entity type
        /// @return identifier_t Entity id or zero if reservation failed
        virtual identifier_t ReserveEntity(const std::string& type) = 0;

        /// @brief Update entity without pushing to history
        ///
        /// @param id Entity id
        /// @param name Entity name
        /// @param scriptSourceId Entity script source id
        /// @param attributes Entity attributes
        /// @return Successfulness
        virtual bool UpdateEntity(identifier_t id, const std::string& name, identifier_t scriptSourceId,
                                  const std::string_view& attributes) = 0;

        /// @brief Update entity state
        ///
        /// @param id Entity data
        /// @param stat State
        /// @return Successfulness
        virtual bool UpdateEntityState(identifier_t id, const std::string_view& state) = 0;

        /// @brief Remove entity
        ///
        /// @param id Entity id
        /// @return Successfulness
        virtual bool RemoveEntity(identifier_t id) = 0;

        /// @brief Get entity count
        ///
        /// @return size_t Entity count
        virtual size_t GetEntityCount() = 0;

        //! User

        /// @brief Load users from database
        ///
        /// @param callback Callback for each user
        /// @return Successfulness
        virtual bool LoadUsers(
            const boost::function<void(identifier_t id, const std::string& name, uint8_t hash[SHA256_SIZE],
                                       uint8_t salt[SALT_SIZE], const std::string& accessLevel)>& callback) = 0;

        /// @brief Reserves new user entry in database
        ///
        /// @param name User name
        /// @return Entry identifier or null in case of an error
        virtual identifier_t ReserveUser(const std::string& name) = 0;

        /// @brief Update user access level
        ///
        /// @param id User id
        /// @param newValue New access level
        /// @return Successfulness
        virtual bool UpdateUserAccessLevel(identifier_t id, const std::string& newValue) = 0;

        /// @brief Update user hash and salt
        ///
        /// @param id User id
        /// @param hash New hash
        /// @param salt New salt
        /// @return Successfulness
        virtual bool UpdateUserHash(identifier_t id, uint8_t hash[SHA256_SIZE], uint8_t salt[SALT_SIZE]) = 0;

        /// @brief Remove user
        ///
        /// @param id User id
        /// @return Successfulness
        virtual bool RemoveUser(identifier_t id) = 0;

        /// @brief Get user count
        ///
        /// @return size_t User count
        virtual size_t GetUserCount() = 0;
    };
}