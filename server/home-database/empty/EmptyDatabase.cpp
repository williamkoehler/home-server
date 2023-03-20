#include "EmptyDatabase.hpp"

namespace server
{
    EmptyDatabase::EmptyDatabase()
    {
    }
    EmptyDatabase::~EmptyDatabase()
    {
    }
    Ref<EmptyDatabase> EmptyDatabase::Create()
    {
        return boost::make_shared<EmptyDatabase>();
    }

    bool EmptyDatabase::LoadScriptSources(
        const boost::function<void(identifier_t id, const std::string& type, const std::string& name,
                                   const std::string& usage, const std::string_view& data)>& callback)
    {
        (void)callback;
        return true;
    }
    identifier_t EmptyDatabase::ReserveScriptSource()
    {
        return ++scriptSourceIDCounter;
    }
    bool EmptyDatabase::UpdateScriptSource(identifier_t id, const std::string& type, const std::string& name,
                                           const std::string& usage, const std::string_view& data)
    {
        (void)id;
        (void)type;
        (void)name;
        (void)usage;
        (void)data;
        return true;
    }
    bool EmptyDatabase::UpdateScriptSourcePropName(identifier_t id, const std::string& value,
                                                   const std::string& newValue)
    {
        (void)id;
        (void)value;
        (void)newValue;
        return true;
    }
    bool EmptyDatabase::UpdateScriptSourcePropContent(identifier_t id, const std::string_view& newValue)
    {
        (void)id;
        (void)newValue;
        return true;
    }
    bool EmptyDatabase::RemoveScriptSource(identifier_t id)
    {
        (void)id;
        return true;
    }
    size_t EmptyDatabase::GetScriptSourceCount()
    {
        return 0;
    }

    bool EmptyDatabase::LoadRooms(
        const boost::function<bool(identifier_t id, const std::string& type, const std::string& name)>& callback)
    {
        (void)callback;
        return 0;
    }
    identifier_t EmptyDatabase::ReserveRoom()
    {
        return ++roomIDCounter;
    }
    bool EmptyDatabase::UpdateRoom(identifier_t id, const std::string& type, const std::string& name)
    {
        (void)id;
        (void)type;
        (void)name;

        return true;
    }
    bool EmptyDatabase::UpdateRoomPropName(identifier_t id, const std::string& value, const std::string& newValue)
    {
        (void)id;
        (void)value;
        (void)newValue;
        return true;
    }
    bool EmptyDatabase::UpdateRoomPropType(identifier_t id, const std::string& value, const std::string& newValue)
    {
        (void)id;
        (void)value;
        (void)newValue;
        return true;
    }
    bool EmptyDatabase::RemoveRoom(identifier_t id)
    {
        (void)id;
        return true;
    }
    size_t EmptyDatabase::GetRoomCount()
    {
        return 0;
    }

    bool EmptyDatabase::LoadDevices(
        const boost::function<bool(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                                   identifier_t roomID, const std::string_view& data)>& callback)
    {
        (void)callback;
        return true;
    }
    identifier_t EmptyDatabase::ReserveDevice()
    {
        return ++deviceIDCounter;
    }
    bool EmptyDatabase::UpdateDevice(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                                     identifier_t roomID)
    {
        (void)id;
        (void)name;
        (void)scriptSourceID;
        (void)roomID;
        return true;
    }
    bool EmptyDatabase::UpdateDevicePropName(identifier_t id, const std::string& value, const std::string& newValue)
    {
        (void)id;
        (void)value;
        (void)newValue;
        return true;
    }
    bool EmptyDatabase::UpdateDevicePropScriptSource(identifier_t id, identifier_t newValue)
    {
        (void)id;
        (void)newValue;
        return true;
    }
    bool EmptyDatabase::UpdateDevicePropRoom(identifier_t id, identifier_t value, identifier_t newValue)
    {
        (void)id;
        (void)value;
        (void)newValue;
        return true;
    }
    bool EmptyDatabase::RemoveDevice(identifier_t id)
    {
        (void)id;
        return true;
    }
    size_t EmptyDatabase::GetDeviceCount()
    {
        return 0;
    }

    bool EmptyDatabase::LoadServices(
        const boost::function<bool(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                                   const std::string_view& data)>& callback)
    {
        (void)callback;
        return true;
    }
    identifier_t EmptyDatabase::ReserveService()
    {
        return ++serviceIDCounter;
    }
    bool EmptyDatabase::UpdateService(identifier_t id, const std::string& name, identifier_t scriptSourceID)
    {
        (void)id;
        (void)name;
        (void)scriptSourceID;
        return true;
    }
    bool EmptyDatabase::UpdateServicePropName(identifier_t id, const std::string& value, const std::string& newValue)
    {
        (void)id;
        (void)value;
        (void)newValue;
        return true;
    }
    bool EmptyDatabase::UpdateServicePropScriptSource(identifier_t id, identifier_t newValue)
    {
        (void)id;
        (void)newValue;
        return true;
    }
    bool EmptyDatabase::RemoveService(identifier_t id)
    {
        (void)id;
        return true;
    }
    size_t EmptyDatabase::GetServiceCount()
    {
        return 0;
    }

    bool EmptyDatabase::LoadUsers(
        const boost::function<void(identifier_t id, const std::string& name, uint8_t hash[SHA256_SIZE],
                                   uint8_t salt[SALT_SIZE], const std::string& accessLevel)>& callback)
    {
        (void)callback;
        return true;
    }
    identifier_t EmptyDatabase::ReserveUser()
    {
        return ++userIDCounter;
    }
    bool EmptyDatabase::UpdateUser(identifier_t id, const std::string& name, uint8_t hash[SHA256_SIZE],
                                   uint8_t salt[SALT_SIZE], const std::string& accessLevel)
    {
        (void)id;
        (void)name;
        (void)hash;
        (void)salt;
        (void)accessLevel;
        return true;
    }
    bool EmptyDatabase::UpdateUserPropName(identifier_t id, const std::string& newValue)
    {
        (void)id;
        (void)newValue;
        return true;
    }
    bool EmptyDatabase::UpdateUserPropAccessLevel(identifier_t id, const std::string& newValue)
    {
        (void)id;
        (void)newValue;
        return true;
    }
    bool EmptyDatabase::UpdateUserPropHash(identifier_t id, uint8_t newValue[SHA256_SIZE])
    {
        (void)id;
        (void)newValue;
        return true;
    }
    bool EmptyDatabase::UpdateUserPropSalt(identifier_t id, uint8_t newValue[SALT_SIZE])
    {
        (void)id;
        (void)newValue;
        return true;
    }
    bool EmptyDatabase::RemoveUser(identifier_t id)
    {
        (void)id;
        return true;
    }
    size_t EmptyDatabase::GetUserCount()
    {
        return 0;
    }
}