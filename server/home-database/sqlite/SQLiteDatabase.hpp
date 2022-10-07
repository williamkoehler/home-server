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
        static Ref<SQLiteDatabase> Create(const std::string& db = "home.sqlite3");

        /// @brief Load script sources from database
        ///
        /// @param callback Entry callback called once for every entry
        /// @return Successfulness
        virtual bool LoadScriptSources(
            const boost::function<void(identifier_t id, const std::string& type, const std::string& name,
                                       const std::string& usage, const std::string_view& data)>& callback) override;

        /// @brief Reserve script source id
        ///
        /// @return identifier_t Script source id
        virtual identifier_t ReserveScriptSource() override;

        /// @brief Update script source
        ///
        /// @param id Script source id
        /// @param type Type
        /// @param name Name
        /// @param usage Usage
        /// @param data Data
        /// @return Successfulness
        virtual bool UpdateScriptSource(identifier_t id, const std::string& type, const std::string& name,
                                        const std::string& usage, const std::string_view& data) override;

        /// @brief Update script source name
        ///
        /// @param id Script source id
        /// @param value Old name
        /// @param newValue New name
        /// @return Successfulness
        virtual bool UpdateScriptSourcePropName(identifier_t id, const std::string& value,
                                                const std::string& newValue) override;

        /// @brief Update script source content
        ///
        /// @param id Script source id
        /// @param newValue New content
        /// @return Successfulness
        virtual bool UpdateScriptSourcePropContent(identifier_t id, const std::string_view& newValue) override;

        /// @brief Remove script source
        ///
        /// @param id Script source id
        /// @return Successfulness
        virtual bool RemoveScriptSource(identifier_t id) override;

        virtual size_t GetScriptSourceCount() override;

        //! Room

        /// @brief Load rooms from database
        /// @param callback Callback for each room
        /// @return Successfulness
        virtual bool LoadRooms(const boost::function<bool(identifier_t id, const std::string& type,
                                                          const std::string& name)>& callback) override;

        /// @brief Reserves new room entry in database
        /// @return Entry identifier or null in case of an error
        virtual identifier_t ReserveRoom() override;

        /// @brief Update room without pushing record
        /// @param room Room to update
        /// @return Successfulness
        virtual bool UpdateRoom(identifier_t id, const std::string& type, const std::string& name) override;

        /// @brief Update room name
        /// @param room Room to update
        /// @param value Old name (for record)
        /// @param newValue New name
        /// @return Successfulness
        virtual bool UpdateRoomPropName(identifier_t id, const std::string& value,
                                        const std::string& newValue) override;
        virtual bool UpdateRoomPropType(identifier_t id, const std::string& value,
                                        const std::string& newValue) override;
        virtual bool RemoveRoom(identifier_t id) override;

        virtual size_t GetRoomCount() override;

        //! Device

        /// @brief Load device from database
        /// @param callback Callback for each device
        /// @return Successfulness
        virtual bool LoadDevices(
            const boost::function<bool(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                                       identifier_t roomID, const std::string_view& data)>& callback) override;

        /// @brief Reserves new device entry in database
        /// @return Entry identifier or null in case of an error
        virtual identifier_t ReserveDevice() override;

        virtual bool UpdateDevice(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                                  identifier_t roomID) override;

        virtual bool UpdateDevicePropName(identifier_t id, const std::string& value,
                                          const std::string& newValue) override;
        virtual bool UpdateDevicePropScriptSource(identifier_t id, identifier_t newValue) override;
        virtual bool UpdateDevicePropRoom(identifier_t id, identifier_t value, identifier_t newValue) override;

        virtual bool RemoveDevice(identifier_t id) override;

        virtual size_t GetDeviceCount() override;

        //! User

        /// @brief Load users from database
        /// @param callback Callback for each user
        /// @return Successfulness
        virtual bool LoadUsers(
            const boost::function<void(identifier_t id, const std::string& name, uint8_t hash[SHA256_SIZE],
                                       uint8_t salt[SALT_SIZE], const std::string& accessLevel)>& callback) override;

        /// @brief Reserves new user entry in database
        /// @return Entry identifier or null in case of an error
        virtual identifier_t ReserveUser() override;

        virtual bool UpdateUser(identifier_t id, const std::string& name, uint8_t hash[SHA256_SIZE],
                                uint8_t salt[SALT_SIZE], const std::string& accessLevel) override;

        /// @brief Update user name
        /// @param script User to update
        /// @param value Old name (for record)
        /// @param newValue New name
        /// @return Successfulness
        virtual bool UpdateUserPropName(identifier_t id, const std::string& newValue) override;
        virtual bool UpdateUserPropAccessLevel(identifier_t id, const std::string& newValue) override;
        virtual bool UpdateUserPropHash(identifier_t id, uint8_t newValue[SHA256_SIZE]) override;
        virtual bool UpdateUserPropSalt(identifier_t id, uint8_t newValue[SALT_SIZE]) override;
        virtual bool RemoveUser(identifier_t id) override;

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