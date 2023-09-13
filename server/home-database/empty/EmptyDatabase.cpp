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
        const boost::function<void(identifier_t id, const std::string& language, const std::string& name,
                                   const std::string_view& config, const std::string_view& content)>& callback)
    {
        (void)callback;
        return true;
    }

    identifier_t EmptyDatabase::ReserveScriptSource(const std::string& language)
    {
        (void)language;
        return ++scriptSourceIdCounter;
    }

    bool EmptyDatabase::UpdateScriptSource(identifier_t id, const std::string& name, const std::string_view& config)
    {
        (void)id;
        (void)name;
        (void)config;
        return true;
    }

    bool EmptyDatabase::UpdateScriptSourceContent(identifier_t id, const std::string_view& newValue)
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

    bool EmptyDatabase::LoadEntities(
        const boost::function<bool(identifier_t id, const std::string& entityType, const std::string& name,
                                   identifier_t scriptSourceID, const std::string_view& attributes,
                                   const std::string_view& state)>& callback)
    {
        (void)callback;
        return true;
    }

    identifier_t EmptyDatabase::ReserveEntity(const std::string& type)
    {
        (void)type;
        return ++entityIdCounter;
    }

    bool EmptyDatabase::UpdateEntity(identifier_t id, const std::string& name, identifier_t scriptSourceId,
                                     const std::string_view& attributes)
    {
        (void)id;
        (void)name;
        (void)scriptSourceId;
        (void)attributes;
        return true;
    }

    bool EmptyDatabase::UpdateEntityState(identifier_t id, const std::string_view& state)
    {
        (void)id;
        (void)state;
        return true;
    }

    bool EmptyDatabase::RemoveEntity(identifier_t id)
    {
        (void)id;
        return true;
    }

    size_t EmptyDatabase::GetEntityCount()
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
    identifier_t EmptyDatabase::ReserveUser(const std::string& name)
    {
        (void)name;
        return ++userIdCounter;
    }
    bool EmptyDatabase::UpdateUserAccessLevel(identifier_t id, const std::string& newValue)
    {
        (void)id;
        (void)newValue;
        return true;
    }
    bool EmptyDatabase::UpdateUserHash(identifier_t id, uint8_t hash[SHA256_SIZE], uint8_t salt[SALT_SIZE])
    {
        (void)id;
        (void)hash;
        (void)salt;
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