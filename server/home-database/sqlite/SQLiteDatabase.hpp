#pragma once
#include "../Database.hpp"
#include "../common.hpp"
#include <sqlite3.h>

namespace server
{
    class SQLiteDatabase : public Database
    {
      private:
        sqlite3* connection;

      public:
        SQLiteDatabase();
        virtual ~SQLiteDatabase();

        /// @brief Create SQLite database instance
        ///
        /// @param db Database location
        /// @return SQLite database
        static Ref<SQLiteDatabase> Create(std::string db = "");

        /// @brief Load script sources from database
        ///
        /// @param callback Entry callback called once for every entry
        /// @return Successfulness
        virtual bool LoadScriptSources(
            const boost::function<void(identifier_t id, const std::string& language, const std::string& name,
                                       const std::string_view& config, const std::string_view& content)>& callback)
            override;

        /// @brief Reserve script source entry in database
        ///
        /// @param language Script source language
        /// @return identifier_t Script source id or zero if reservation failed
        virtual identifier_t ReserveScriptSource(const std::string& language) override;

        /// @brief Update script source
        ///
        /// @param id Script source id
        /// @param language Script source language
        /// @param name Script source name
        /// @param content Additional config
        /// @return Successfulness
        virtual bool UpdateScriptSource(identifier_t id, const std::string& name,
                                        const std::string_view& config) override;

        /// @brief Update script source content
        ///
        /// @param id Script source id
        /// @param newValue New content
        /// @return Successfulness
        virtual bool UpdateScriptSourceContent(identifier_t id, const std::string_view& newValue) override;

        /// @brief Remove script source
        ///
        /// @param id Script source id
        /// @return Successfulness
        virtual bool RemoveScriptSource(identifier_t id) override;

        /// @brief Get script source count
        ///
        /// @return size_t Script source count
        virtual size_t GetScriptSourceCount() override;

        //! Entity

        /// @brief Load entities from database
        ///
        /// @param callback Entry callback
        /// @return Successfulness
        virtual bool LoadEntities(
            const boost::function<bool(identifier_t id, const std::string& type, const std::string& name,
                                       identifier_t scriptSourceID, const std::string_view& attributes,
                                       const std::string_view& state)>& callback) override;

        /// @brief Reserve entity entry in database
        ///
        /// @param type Entity type
        /// @return identifier_t Entity id or zero if reservation failed
        virtual identifier_t ReserveEntity(const std::string& type) override;

        /// @brief Update entity without pushing to history
        ///
        /// @param id Entity id
        /// @param name Entity name
        /// @param scriptSourceId Entity script source id
        /// @param attributes Entity attributes
        /// @return Successfulness
        virtual bool UpdateEntity(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                                  const std::string_view& attributes) override;

        /// @brief Update entity script data
        ///
        /// @param id Entity data
        /// @param data Script data
        /// @return Successfulness
        virtual bool UpdateEntityState(identifier_t id, const std::string_view& state) override;

        /// @brief Remove entity
        ///
        /// @param id Entity id
        /// @return Successfulness
        virtual bool RemoveEntity(identifier_t id) override;

        /// @brief Get entity count
        ///
        /// @return size_t Entity count
        virtual size_t GetEntityCount() override;

        //! User

        /// @brief Load users from database
        ///
        /// @param callback Callback for each user
        /// @return Successfulness
        virtual bool LoadUsers(
            const boost::function<void(identifier_t id, const std::string& name, uint8_t hash[SHA256_SIZE],
                                       uint8_t salt[SALT_SIZE], const std::string& accessLevel)>& callback) override;

        /// @brief Reserves new user entry in database
        ///
        /// @param name User name
        /// @return Entry identifier or null in case of an error
        virtual identifier_t ReserveUser(const std::string& name) override;

        /// @brief Update user access level
        ///
        /// @param id User id
        /// @param newValue New access level
        /// @return Successfulness
        virtual bool UpdateUserAccessLevel(identifier_t id, const std::string& newValue) override;

        /// @brief Update user hash and salt
        ///
        /// @param id User id
        /// @param hash New hash
        /// @param salt New salt
        /// @return Successfulness
        virtual bool UpdateUserHash(identifier_t id, uint8_t hash[SHA256_SIZE], uint8_t salt[SALT_SIZE]) override;

        /// @brief Remove user
        ///
        /// @param id User id
        /// @return Successfulness
        virtual bool RemoveUser(identifier_t id) override;

        /// @brief Get user count
        ///
        /// @return size_t User count
        virtual size_t GetUserCount() override;

        //! Testing

        /// @brief Only used in tests
        /// @return SQLite Database connection
        inline sqlite3* GetConnection_TEST() const
        {
            return connection;
        }
    };
}