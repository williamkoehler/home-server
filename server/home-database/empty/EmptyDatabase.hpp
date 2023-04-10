#pragma once
#include "../Database.hpp"
#include "../common.hpp"

namespace server
{
    class EmptyDatabase : public Database
    {
      private:
        identifier_t scriptSourceIdCounter;
        identifier_t entityIdCounter;
        identifier_t userIdCounter;

      public:
        EmptyDatabase();
        virtual ~EmptyDatabase();
        static Ref<EmptyDatabase> Create();

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
            const boost::function<bool(identifier_t id, const std::string& entityType, const std::string& name,
                                       identifier_t scriptSourceID, const std::string_view& data,
                                       const std::string_view& scriptData)>& callback) override;

        /// @brief Reserve entity entry in database
        ///
        /// @param type Entity type
        /// @return identifier_t Entity id or zero if reservation failed
        virtual identifier_t ReserveEntity(const std::string& type) override;

        /// @brief Update entity without pushing to history
        ///
        /// @param id Entity id
        /// @param name Entity name
        /// @param data Data
        /// @param scriptData Script data
        /// @return Successfulness
        virtual bool UpdateEntity(identifier_t id, const std::string& name, identifier_t scriptSourceId,
                                  const std::string_view& config) override;

        /// @brief Update entity state
        ///
        /// @param id Entity data
        /// @param stat State
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
        /// @param callback Callback for each user
        /// @return Successfulness
        virtual bool LoadUsers(
            const boost::function<void(identifier_t id, const std::string& name, uint8_t hash[SHA256_SIZE],
                                       uint8_t salt[SALT_SIZE], const std::string& accessLevel)>& callback) override;

        /// @brief Reserves new user entry in database
        /// @return Entry identifier or null in case of an error
        virtual identifier_t ReserveUser(const std::string& name) override;
        
        virtual bool UpdateUserAccessLevel(identifier_t id, const std::string& newValue) override;
        virtual bool UpdateUserHash(identifier_t id, uint8_t hash[SHA256_SIZE], uint8_t salt[SALT_SIZE]) override;
        virtual bool RemoveUser(identifier_t id) override;

        virtual size_t GetUserCount() override;
    };
}