#include "Entity.hpp"
#include "Device.hpp"
#include "Home.hpp"
#include "Room.hpp"
#include "Service.hpp"
#include <home-database/Database.hpp>
#include <home-scripting/Script.hpp>
#include <home-scripting/ScriptManager.hpp>
#include <home-scripting/ScriptSource.hpp>

namespace server
{
    namespace main
    {
        std::string StringifyEntityType(EntityType type)
        {
            switch (type)
            {
            case EntityType::kRoomEntityType:
                return "room";
            case EntityType::kDeviceEntityType:
                return "device";
            case EntityType::kServiceEntityType:
                return "service";
            default:
                return "unknown";
            }
        }
        EntityType ParseEntityType(const std::string& type)
        {
            switch (crc32(type.data(), type.size()))
            {
            case CRC32("room"):
                return EntityType::kRoomEntityType;
            case CRC32("device"):
                return EntityType::kDeviceEntityType;
            case CRC32("service"):
                return EntityType::kServiceEntityType;
            default:
                return EntityType::kUnknownEntityType;
            }
        }

        uint8_t EntityTypeToScriptFlags(EntityType entityType)
        {
            switch (entityType)
            {
            case EntityType::kRoomEntityType:
                return scripting::ScriptFlags::kScriptFlag_RoomSupport;
            case EntityType::kDeviceEntityType:
                return scripting::ScriptFlags::kScriptFlag_DeviceSupport;
            case EntityType::kServiceEntityType:
                return scripting::ScriptFlags::kScriptFlag_ServiceSupport;
            default:
                return scripting::ScriptFlags::kScriptFlag_None;
            }
        }

        Entity::Entity(identifier_t id, const std::string& name) : id(id), name(name)
        {
        }
        Entity::~Entity()
        {
        }
        Ref<Entity> Entity::Create(identifier_t id, EntityType type, const std::string& name,
                                   identifier_t scriptSourceId, const rapidjson::Value& config,
                                   const rapidjson::Value& state)
        {
            Ref<Entity> entity;

            switch (type)
            {
            case EntityType::kRoomEntityType:
                entity = Room::Create(id, name);
                break;
            case EntityType::kDeviceEntityType:
                entity = Device::Create(id, name);
                break;
            case EntityType::kServiceEntityType:
                entity = Service::Create(id, name);
                break;
            default:
                LOG_ERROR("Invalid entity type.");
                return nullptr;
            }

            if (entity != nullptr)
            {
                // Set config
                entity->JsonSetConfig(config);

                if (scriptSourceId != 0)
                {
                    // Create script
                    Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
                    assert(scriptManager != nullptr);

                    entity->script =
                        scriptManager->CreateScript(scriptSourceId, EntityTypeToScriptFlags(type), entity->GetView());
                    if (entity->script == nullptr)
                    {
                        LOG_ERROR("Create script '{0}'", scriptSourceId);
                        return nullptr;
                    }

                    // Initialize script
                    entity->script->Initialize();

                    // Set state
                    entity->script->JsonSetProperties(state, scripting::PropertyFlags::kPropertyFlags_All);
                }
            }

            return entity;
        }
        Ref<Entity> Entity::Create(identifier_t id, EntityType type, const std::string& name,
                                   identifier_t scriptSourceID, const std::string_view& config,
                                   const std::string_view& state)
        {
            rapidjson::Document configJson, stateJson;

            // Parse data
            {
                rapidjson::StringStream stream = rapidjson::StringStream(config.data());
                configJson.ParseStream(stream);
                if (configJson.HasParseError() || !configJson.IsObject())
                {
                    LOG_WARNING("Failed to parse config of entity '{0}'.", id);
                    configJson = rapidjson::Document(rapidjson::kObjectType);
                }
            }

            // Parse script data
            {
                rapidjson::StringStream stream = rapidjson::StringStream(state.data());
                stateJson.ParseStream(stream);
                if (configJson.HasParseError() || !configJson.IsObject())
                {
                    LOG_WARNING("Failed to parse state of entity '{0}'.", id);
                    stateJson = rapidjson::Document(rapidjson::kObjectType);
                }
            }

            return Create(id, type, name, scriptSourceID, configJson, stateJson);
        }

        void Entity::SetScript(identifier_t scriptSourceId)
        {
            if (scriptSourceId != 0)
            {
                // Create script
                Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
                assert(scriptManager != nullptr);

                EntityType type = GetType();

                script = scriptManager->CreateScript(scriptSourceId, EntityTypeToScriptFlags(type), GetView());
                if (script == nullptr)
                {
                    LOG_ERROR("Failed to create new script from script source '{0}'", scriptSourceId);
                    return;
                }

                // Initialize script
                script->Initialize();
            }
            else
            {
                script = nullptr;
            }
        }

        bool Entity::SaveConfig()
        {
            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Generate json additional config
            rapidjson::StringBuffer config = rapidjson::StringBuffer();
            {
                // Generate json
                rapidjson::Document document = rapidjson::Document(rapidjson::kObjectType);
                JsonGetConfig(document, document.GetAllocator());

                // Stringify json
                rapidjson::Writer<rapidjson::StringBuffer> writer = rapidjson::Writer<rapidjson::StringBuffer>(config);
                document.Accept(writer);
            }

            // Update database
            return database->UpdateEntity(id, name, GetScriptSourceId(),
                                          std::string_view(config.GetString(), config.GetSize()));
        }

        bool Entity::SaveState()
        {
            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Generate json state
            rapidjson::StringBuffer state = rapidjson::StringBuffer();
            {
                // Generate json
                rapidjson::Document document = rapidjson::Document(rapidjson::kObjectType);
                if (script != nullptr)
                    script->JsonGetProperties(document, document.GetAllocator(),
                                              scripting::PropertyFlags::kPropertyFlag_Store);

                // Stringify json
                rapidjson::Writer<rapidjson::StringBuffer> writer = rapidjson::Writer<rapidjson::StringBuffer>(state);
                document.Accept(writer);
            }

            // Update database
            return database->UpdateEntityState(id, std::string_view(state.GetString(), state.GetSize()));
        }

        void Entity::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            output.AddMember("id", rapidjson::Value(id), allocator);

            std::string type = StringifyEntityType(GetType());
            output.AddMember("type", rapidjson::Value(type.data(), type.size(), allocator), allocator);

            output.AddMember("name", rapidjson::Value(name.data(), name.size(), allocator), allocator);

            if (script != nullptr)
            {
                output.AddMember("scriptsourceid", rapidjson::Value(script->GetSourceID()), allocator);

                rapidjson::Value scriptJson = rapidjson::Value(rapidjson::kObjectType);
                script->JsonGet(scriptJson, allocator);
                output.AddMember("script", scriptJson, allocator);
            }
            else
            {
                output.AddMember("scriptsourceid", rapidjson::Value(rapidjson::kNullType), allocator);
                output.AddMember("script", rapidjson::Value(rapidjson::kNullType), allocator);
            }

            JsonGetConfig(output, allocator);
        }
        bool Entity::JsonSet(const rapidjson::Value& input)
        {
            assert(input.IsObject());

            bool update = JsonSetConfig(input);

            rapidjson::Value::ConstMemberIterator nameIt = input.FindMember("name");
            if (nameIt != input.MemberEnd() && nameIt->value.IsString())
            {
                name.assign(nameIt->value.GetString(), nameIt->value.GetStringLength());
                update = true;
            }

            rapidjson::Value::ConstMemberIterator scriptSourceIDIt = input.FindMember("scriptsourceid");
            if (scriptSourceIDIt != input.MemberEnd() && scriptSourceIDIt->value.IsUint())
            {
                SetScript(scriptSourceIDIt->value.GetUint());
                update = true;
            }

            return update;
        }

        void Entity::ApiInvoke(const std::string& method, const scripting::Value& parameter)
        {
            if (script)
                script->PostInvoke(method, parameter);
        }

        void Entity::ApiGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator,
                            ApiContext& context)
        {
            (void)context;

            assert(output.IsObject());

            JsonGet(output, allocator);
        }
        bool Entity::ApiSet(const rapidjson::Value& input, ApiContext& context)
        {
            (void)context;

            assert(input.IsObject());

            bool update = JsonSet(input);

            if (update)
                SaveConfig();

            return update;
        }

        void Entity::ApiGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator,
                                 ApiContext& context)
        {
            (void)context;

            assert(output.IsObject());

            output.AddMember("id", rapidjson::Value(id), allocator);

            rapidjson::Value state = rapidjson::Value(rapidjson::kObjectType);

            if (script != nullptr)
                script->JsonGetProperties(state, allocator, scripting::PropertyFlags::kPropertyFlag_Visible);

            output.AddMember("state", state, allocator);
        }
        bool Entity::ApiSetState(const rapidjson::Value& input, ApiContext& context)
        {
            (void)context;

            assert(input.IsObject());

            if (script == nullptr)
                return false;

            uint8_t updateFlags = script->JsonSetProperties(input);

            if (updateFlags & scripting::PropertyFlags::kPropertyFlag_Store)
                SaveState();

            return updateFlags != 0;
        }
    }
}