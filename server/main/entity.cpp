#include "entity.hpp"
#include "device.hpp"
#include "home.hpp"
#include "room.hpp"
#include "service.hpp"
#include <database/database.hpp>
#include <scripting/script.hpp>
#include <scripting/script_manager.hpp>
#include <scripting/script_source.hpp>

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

        Entity::Entity(identifier_t id, const std::string& name)
            : id(id), name(name), lazyUpdateInterval(1), lazyUpdateTimer(Worker::GetInstance()->GetContext())
        {
        }
        Entity::~Entity()
        {
        }
        Ref<Entity> Entity::Create(identifier_t id, EntityType type, const std::string& name,
                                   identifier_t scriptSourceId, const rapidjson::Value& attributesJson,
                                   const rapidjson::Value& stateJson)
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
                entity->JsonSetAttributes(attributesJson);

                // Set script
                entity->SetScript(scriptSourceId);

                // Set state
                if (entity->script)
                    entity->script->JsonSetProperties(stateJson, scripting::kPropertyFlag_All);
            }

            return entity;
        }
        Ref<Entity> Entity::Create(identifier_t id, EntityType type, const std::string& name,
                                   identifier_t scriptSourceID, const std::string_view& attributes,
                                   const std::string_view& state)
        {
            rapidjson::Document configJson, stateJson;

            // Parse attributes
            {
                rapidjson::StringStream stream = rapidjson::StringStream(attributes.data());
                configJson.ParseStream(stream);
                if (configJson.HasParseError() || !configJson.IsObject())
                {
                    LOG_WARNING("Failed to parse attributes of entity '{0}'.", id);
                    configJson = rapidjson::Document(rapidjson::kObjectType);
                }
            }

            // Parse script state
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

                // Get attributes
                lazyUpdateInterval = script->GetLazyUpdateInterval();
            }
            else
            {
                script = nullptr;
            }
        }

        void Entity::Subscribe(const Ref<api::WebSocketSession>& session)
        {
            if (sessions.AddSession(session))
            {
                if (lazyUpdateInterval > 0)
                {
                    lazyUpdateTimer.expires_from_now(boost::posix_time::seconds(0));
                    lazyUpdateTimer.async_wait(
                        boost::bind(&Entity::WaitLazyUpdateTimer, shared_from_this(), boost::placeholders::_1));
                }
            }
        }

        void Entity::WaitLazyUpdateTimer(const boost::system::error_code& ec)
        {
            if (!ec)
            {
                if (script != nullptr)
                    script->LazyUpdate();

                // Only reset timer if there are subscriptions left
                if (sessions.GetSessionCount() > 0)
                {
                    lazyUpdateTimer.expires_from_now(boost::posix_time::seconds(std::max(lazyUpdateInterval, 1ul)));
                    lazyUpdateTimer.async_wait(
                        boost::bind(&Entity::WaitLazyUpdateTimer, shared_from_this(), boost::placeholders::_1));
                }
            }
        }

        void Entity::Unsubscribe(const Ref<api::WebSocketSession>& session)
        {
            sessions.RemoveSession(session);
        }

        void Entity::Invoke(const std::string& method, const scripting::Value& parameter)
        {
            if (script)
                script->PostInvoke(method, parameter);
        }

        void Entity::Publish()
        {
            api::ApiBroadcastMessage message = api::ApiBroadcastMessage("set-entity");
            JsonGet(message.GetJsonDocument(), message.GetJsonAllocator());

            sessions.Send(message);
        }

        void Entity::PublishState()
        {
            api::ApiBroadcastMessage message = api::ApiBroadcastMessage("set-entity-state");
            {

                rapidjson::Document& output = message.GetJsonDocument();
                rapidjson::Document::AllocatorType& allocator = message.GetJsonAllocator();

                rapidjson::Value stateJson = rapidjson::Value(rapidjson::kObjectType);
                JsonGetState(stateJson, allocator);
                output.AddMember("state", stateJson, allocator);
            }

            sessions.Send(message);
        }

        bool Entity::Save()
        {
            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Generate json additional config
            rapidjson::StringBuffer attributes = rapidjson::StringBuffer();
            {
                // Generate json
                rapidjson::Document attributesJson = rapidjson::Document(rapidjson::kObjectType);
                JsonGetAttributes(attributesJson, attributesJson.GetAllocator());

                // Stringify json
                rapidjson::Writer<rapidjson::StringBuffer> writer =
                    rapidjson::Writer<rapidjson::StringBuffer>(attributes);
                attributesJson.Accept(writer);
            }

            // Update database
            return database->UpdateEntity(id, name, GetScriptSourceId(),
                                          std::string_view(attributes.GetString(), attributes.GetSize()));
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
                    script->JsonGetProperties(document, document.GetAllocator(), scripting::kPropertyFlag_Store);

                // Stringify json
                rapidjson::Writer<rapidjson::StringBuffer> writer = rapidjson::Writer<rapidjson::StringBuffer>(state);
                document.Accept(writer);
            }

            // Update database
            return database->UpdateEntityState(id, std::string_view(state.GetString(), state.GetSize()));
        }

        void Entity::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) const
        {
            assert(output.IsObject());

            output.AddMember("id", rapidjson::Value(id), allocator);

            std::string type = StringifyEntityType(GetType());
            output.AddMember("type", rapidjson::Value(type.data(), type.size(), allocator), allocator);

            output.AddMember("name", rapidjson::Value(name.data(), name.size(), allocator), allocator);

            output.AddMember("scriptsourceid",
                             script != nullptr ? rapidjson::Value(script->GetSourceID())
                                               : rapidjson::Value(rapidjson::kNullType),
                             allocator);

            rapidjson::Value attributesJson = rapidjson::Value(rapidjson::kObjectType);

            if (script != nullptr)
                script->JsonGetAttributes(attributesJson, allocator);
            JsonGetAttributes(attributesJson, allocator);

            output.AddMember("attributes", attributesJson, allocator);
        }
        bool Entity::JsonSet(const rapidjson::Value& input)
        {
            assert(input.IsObject());

            bool update = false;

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

            rapidjson::Value::ConstMemberIterator attributesIt = input.FindMember("attributes");
            if (attributesIt != input.MemberEnd() && attributesIt->value.IsObject())
            {
                JsonSetAttributes(attributesIt->value);
                update = true;
            }

            return update;
        }

        void Entity::JsonGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) const
        {
            assert(output.IsObject());

            if (script != nullptr)
            {
                // Get state
                script->JsonGetProperties(output, allocator);
            }
        }

        bool Entity::JsonSetState(const rapidjson::Value& input)
        {
            assert(input.IsObject());

            if (script != nullptr)
            {
                // Set state
                scripting::PropertyFlags update = script->JsonSetProperties(input);
                if (update & scripting::PropertyFlag::kPropertyFlag_Store)
                    SaveState();

                return update != 0;
            }

            return false;
        }
    }
}