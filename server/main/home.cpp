#include "home.hpp"
#include "device.hpp"
#include "entity.hpp"
#include "room.hpp"
#include "service.hpp"
#include <database/database.hpp>
#include <scripting/script_manager.hpp>

namespace server
{
    namespace main
    {
        WeakRef<Home> instanceHome;

        Home::Home()
        {
        }
        Home::~Home()
        {
            entityMap.clear();
        }
        Ref<Home> Home::Create()
        {
            if (!instanceHome.expired())
                return Ref<Home>(instanceHome);

            Ref<Home> home = boost::make_shared<Home>();
            if (home == nullptr)
                return nullptr;

            instanceHome = home;

            // Load from database
            {
                Ref<Database> database = Database::GetInstance();
                assert(database != nullptr);

                // Load entities
                if (!database->LoadEntities(boost::bind(&Home::LoadEntity, home, boost::placeholders::_1,
                                                        boost::placeholders::_2, boost::placeholders::_3,
                                                        boost::placeholders::_4, boost::placeholders::_5,
                                                        boost::placeholders::_6)))
                {
                    LOG_ERROR("Loading rooms.");
                    return nullptr;
                }
            }

            // Create home view
            home->view = boost::make_shared<HomeView>(home);
            if (home->view == nullptr)
            {
                LOG_ERROR("Create home view.");
                return nullptr;
            }

            // Set home view
            scripting::ScriptManager::SetHomeView(home->view);

            home->UpdateTimestamp();

            // Register websocket home api
            {
                robin_hood::unordered_node_map<std::string, api::WebSocketApiCallDefinition>& apiMap =
                    api::WebSocketSession::GetApiMap();

                apiMap["get-home"] = Home::WebSocketProcessGetHomeMessage;

                apiMap["add-entity"] = Home::WebSocketProcessAddEntityMessage;
                apiMap["rem-entity"] = Home::WebSocketProcessRemoveEntityMessage;

                apiMap["get-entity"] = Home::WebSocketProcessGetEntityMessage;
                apiMap["set-entity"] = Home::WebSocketProcessSetEntityMessage;

                apiMap["get-entity-state"] = Home::WebSocketProcessGetEntityStateMessage;
                apiMap["set-entity-state"] = Home::WebSocketProcessSetEntityStateMessage;

                apiMap["inv-entiy"] = Home::WebSocketProcessInvokeDeviceMethodMessage;

                apiMap["sub-to-entity-state"] = Home::WebSocketProcessSubscribeToEntityStateMessage;
                apiMap["unsub-from-entity-state"] = Home::WebSocketProcessUnsubscribeFromEntityStateMessage;
            }

            return home;
        }
        Ref<Home> Home::GetInstance()
        {
            return Ref<Home>(instanceHome);
        }

        //! Timestamp
        void Home::UpdateTimestamp()
        {
            const time_t ts = time(nullptr);
            timestamp = ts;
        }

        bool Home::LoadEntity(identifier_t id, const std::string& type, const std::string& name,
                              identifier_t scriptSourceId, const std::string_view& attributes,
                              const std::string_view& state)
        {
            // Create new entity
            Ref<Entity> entity = Entity::Create(id, ParseEntityType(type), name, scriptSourceId, attributes, state);

            // Add entity
            if (entity != nullptr)
            {
                entityMap[entity->GetID()] = entity;

                return true;
            }
            else
                return false;
        }

        Ref<Entity> Home::AddEntity(EntityType type, const std::string& name, identifier_t scriptSourceId,
                                    const rapidjson::Value& attributesJson)
        {
            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Reserve room in database
            identifier_t id = database->ReserveEntity(StringifyEntityType(type));
            if (id == 0)
                return nullptr;

            // Create new entity
            Ref<Entity> entity =
                Entity::Create(id, type, name, scriptSourceId, attributesJson, rapidjson::Value(rapidjson::kObjectType));

            // Add entity
            if (entity != nullptr)
            {
                // Save entity config and state
                entity->Save();
                entity->SaveState();

                entityMap[entity->GetID()] = entity;
            }
            else
            {
                database->RemoveEntity(id);
                return nullptr;
            }

            UpdateTimestamp();

            return entity;
        }

        Ref<Entity> Home::GetEntity(identifier_t entityId)
        {
            const robin_hood::unordered_node_map<identifier_t, Ref<Entity>>::const_iterator it =
                entityMap.find(entityId);
            if (it == entityMap.end())
                return nullptr;

            return (*it).second;
        }

        Ref<Room> Home::GetRoom(identifier_t entityId)
        {
            const robin_hood::unordered_node_map<identifier_t, Ref<Entity>>::const_iterator it =
                entityMap.find(entityId);
            if (it == entityMap.end())
                return nullptr;

            return boost::dynamic_pointer_cast<Room>((*it).second);
        }

        Ref<Device> Home::GetDevice(identifier_t entityId)
        {
            const robin_hood::unordered_node_map<identifier_t, Ref<Entity>>::const_iterator it =
                entityMap.find(entityId);
            if (it == entityMap.end())
                return nullptr;

            return boost::dynamic_pointer_cast<Device>((*it).second);
        }

        Ref<Service> Home::GetService(identifier_t entityId)
        {
            const robin_hood::unordered_node_map<identifier_t, Ref<Entity>>::const_iterator it =
                entityMap.find(entityId);
            if (it == entityMap.end())
                return nullptr;

            return boost::dynamic_pointer_cast<Service>((*it).second);
        }

        bool Home::RemoveEntity(identifier_t entityId)
        {
            if (entityMap.erase(entityId))
            {
                Ref<Database> database = Database::GetInstance();
                assert(database != nullptr);

                database->RemoveEntity(entityId);

                return true;
            }
            else
                return false;
        }

        void Home::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) const
        {
            output.AddMember("timestamp", rapidjson::Value(timestamp), allocator);

            rapidjson::Value entitiesJson = rapidjson::Value(rapidjson::kArrayType);
            entitiesJson.Reserve(entityMap.size(), allocator);

            for (const auto& [id, entity] : entityMap)
            {
                assert(entity != nullptr);

                rapidjson::Value entityJson = rapidjson::Value(rapidjson::kObjectType);
                entity->JsonGet(entityJson, allocator);

                entitiesJson.PushBack(entityJson, allocator);
            }

            output.AddMember("entities", entitiesJson, allocator);
        }

        HomeView::HomeView(const Ref<Home>& home) : home(home)
        {
        }
        HomeView::~HomeView()
        {
        }

        Ref<scripting::RoomView> HomeView::GetRoom(identifier_t id) const
        {
            Ref<Home> r = home.lock();
            assert(r != nullptr);

            // Get room using identifier
            Ref<Room> room = r->GetRoom(id);
            if (room != nullptr)
                return room->GetRoomView();
            else
                return nullptr;
        }

        Ref<scripting::DeviceView> HomeView::GetDevice(identifier_t id) const
        {
            Ref<Home> r = home.lock();
            assert(r != nullptr);

            // Get device using identifier
            Ref<Device> device = r->GetDevice(id);
            if (device != nullptr)
                return device->GetDeviceView();
            else
                return nullptr;
        }
    }
}