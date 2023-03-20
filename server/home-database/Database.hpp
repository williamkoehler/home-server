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
            const boost::function<void(identifier_t id, const std::string& type, const std::string& name,
                                       const std::string& usage, const std::string_view& content)>& callback) = 0;

        /// @brief Reserve script source id
        ///
        /// @return identifier_t Script source id or 0 in case of an error
        virtual identifier_t ReserveScriptSource() = 0;

        /// @brief Update script source
        ///
        /// @param id Script source id
        /// @param type Type
        /// @param name Name
        /// @param usage Usage
        /// @param content Content
        /// @return Successfulness
        virtual bool UpdateScriptSource(identifier_t id, const std::string& type, const std::string& name,
                                        const std::string& usage, const std::string_view& content) = 0;

        /// @brief Update script source name
        ///
        /// @param id Script source id
        /// @param value Old name
        /// @param newValue New name
        /// @return Successfulness
        virtual bool UpdateScriptSourcePropName(identifier_t id, const std::string& value,
                                                const std::string& newValue) = 0;

        /// @brief Update script source content
        ///
        /// @param id Script source id
        /// @param newValue New content
        /// @return Successfulness
        virtual bool UpdateScriptSourcePropContent(identifier_t id, const std::string_view& newValue) = 0;

        /// @brief Remove script source
        ///
        /// @param id Script source id
        /// @return Successfulness
        virtual bool RemoveScriptSource(identifier_t id) = 0;

        virtual size_t GetScriptSourceCount() = 0;

        //! Room

        /// @brief Load rooms from database
        /// @param callback Callback for each room
        /// @return Successfulness
        virtual bool LoadRooms(const boost::function<bool(identifier_t id, const std::string& type,
                                                          const std::string& name)>& callback) = 0;

        /// @brief Reserves new room entry in database
        /// @return Entry identifier or 0 in case of an error
        virtual identifier_t ReserveRoom() = 0;

        /// @brief Update room without pushing record
        /// @param room Room to update
        /// @return Successfulness
        virtual bool UpdateRoom(identifier_t id, const std::string& type, const std::string& name) = 0;

        /// @brief Update room name
        /// @param room Room to update
        /// @param value Old name (for record)
        /// @param newValue New name
        /// @return Successfulness
        virtual bool UpdateRoomPropName(identifier_t id, const std::string& value, const std::string& newValue) = 0;
        virtual bool UpdateRoomPropType(identifier_t id, const std::string& value, const std::string& newValue) = 0;
        virtual bool RemoveRoom(identifier_t id) = 0;

        virtual size_t GetRoomCount() = 0;

        //! Device

        /// @brief Load device from database
        /// @param callback Callback for each device
        /// @return Successfulness
        virtual bool LoadDevices(
            const boost::function<bool(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                                       identifier_t roomID, const std::string_view& data)>& callback) = 0;

        /// @brief Reserves new device entry in database
        /// @return Entry identifier or 0 in case of an error
        virtual identifier_t ReserveDevice() = 0;

        virtual bool UpdateDevice(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                                  identifier_t roomID) = 0;

        virtual bool UpdateDevicePropName(identifier_t id, const std::string& value, const std::string& newValue) = 0;
        virtual bool UpdateDevicePropScriptSource(identifier_t id, identifier_t newValue) = 0;
        virtual bool UpdateDevicePropRoom(identifier_t id, identifier_t value, identifier_t newValue) = 0;

        virtual bool RemoveDevice(identifier_t id) = 0;

        virtual size_t GetDeviceCount() = 0;

        //! Service

        /// @brief Load service from database
        /// @param callback Callback for each service
        /// @return Successfulness
        virtual bool LoadServices(
            const boost::function<bool(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                                       const std::string_view& data)>& callback) = 0;

        /// @brief Reserves new service entry in database
        /// @return Entry identifier or 0 in case of an error
        virtual identifier_t ReserveService() = 0;

        virtual bool UpdateService(identifier_t id, const std::string& name, identifier_t scriptSourceID) = 0;

        virtual bool UpdateServicePropName(identifier_t id, const std::string& value, const std::string& newValue) = 0;
        virtual bool UpdateServicePropScriptSource(identifier_t id, identifier_t newValue) = 0;

        virtual bool RemoveService(identifier_t id) = 0;

        virtual size_t GetServiceCount() = 0;

        //! User

        /// @brief Load users from database
        /// @param callback Callback for each user
        /// @return Successfulness
        virtual bool LoadUsers(
            const boost::function<void(identifier_t id, const std::string& name, uint8_t hash[SHA256_SIZE],
                                       uint8_t salt[SALT_SIZE], const std::string& accessLevel)>& callback) = 0;

        /// @brief Reserves new user entry in database
        /// @return Entry identifier or null in case of an error
        virtual identifier_t ReserveUser() = 0;

        virtual bool UpdateUser(identifier_t id, const std::string& name, uint8_t hash[SHA256_SIZE],
                                uint8_t salt[SALT_SIZE], const std::string& accessLevel) = 0;

        /// @brief Update user name
        /// @param script User to update
        /// @param value Old name (for record)
        /// @param newValue New name
        /// @return Successfulness
        virtual bool UpdateUserPropName(identifier_t id, const std::string& newValue) = 0;
        virtual bool UpdateUserPropAccessLevel(identifier_t id, const std::string& newValue) = 0;
        virtual bool UpdateUserPropHash(identifier_t id, uint8_t newValue[SHA256_SIZE]) = 0;
        virtual bool UpdateUserPropSalt(identifier_t id, uint8_t newValue[SALT_SIZE]) = 0;
        virtual bool RemoveUser(identifier_t id) = 0;

        virtual size_t GetUserCount() = 0;
    };
}