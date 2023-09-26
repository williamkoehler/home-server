#pragma once
#include "common.hpp"
#include <api/websocket_session_set.hpp>
#include <scripting/script.hpp>
#include <scripting_sdk/view/view.hpp>

namespace server
{
    namespace main
    {
        class Room;
        class Device;
        class Service;

        enum class EntityType
        {
            kUnknownEntityType,
            kRoomEntityType,
            kDeviceEntityType,
            kServiceEntityType,
        };

        std::string StringifyEntityType(EntityType type);
        EntityType ParseEntityType(const std::string& type);

        class Entity : public boost::enable_shared_from_this<Entity>
        {
          protected:
            const identifier_t id;
            std::string name;

            Ref<scripting::Script> script;

            // A lazy update timer is executed when there is at least one subscription
            size_t lazyUpdateInterval;
            boost::asio::deadline_timer lazyUpdateTimer;

            void WaitLazyUpdateTimer(const boost::system::error_code& ec);

            /// @brief Update subscriptions
            ///
            api::WebSocketSessionSet sessions;

          public:
            Entity(identifier_t id, const std::string& name);
            virtual ~Entity();
            static Ref<Entity> Create(identifier_t id, EntityType type, const std::string& name,
                                      identifier_t scriptSourceID, const rapidjson::Value& attributesJson,
                                      const rapidjson::Value& stateJson);
            static Ref<Entity> Create(identifier_t id, EntityType type, const std::string& name,
                                      identifier_t scriptSourceID, const std::string_view& attributes,
                                      const std::string_view& state);

            /// @brief Get entity type
            ///
            /// @return EntityType Entity type
            virtual EntityType GetType() const = 0;

            /// @brief Get entity id
            ///
            /// @return Entity id
            inline identifier_t GetID()
            {
                return id;
            }

            /// @brief Get entity name
            ///
            /// @return Entity name
            std::string GetName() const
            {
                return name;
            }

            /// @brief Set entity name
            ///
            /// @param v New entity name
            void SetName(const std::string& v)
            {
                name = v;
            }

            /// @brief Get script
            ///
            /// @return Ref<scripting::Script> Script or null
            inline Ref<scripting::Script> GetScript() const
            {
                return script;
            }

            /// @brief Set script
            ///
            /// @param scriptSourceId Script source id
            void SetScript(identifier_t scriptSourceId);

            /// @brief Get script source id
            ///
            /// @return identifier_t Script source id
            identifier_t GetScriptSourceId() const
            {
                return script != nullptr ? script->GetSourceID() : 0;
            }

            /// @brief Get view
            ///
            /// @return Ref<scripting::sdk::View> Get view of this object
            virtual Ref<scripting::sdk::View> GetView() = 0;

            /// @brief Invoke script method
            ///
            /// @param event Method name
            void Invoke(const std::string& method, const scripting::sdk::Value& parameter);

            /// @brief Make session subscribe to entity
            ///
            /// @param session Api session
            void Subscribe(const Ref<api::WebSocketSession>& session);

            /// @brief Make session unsubscribe from entity
            ///
            /// @param session Api session
            void Unsubscribe(const Ref<api::WebSocketSession>& session);

            /// @brief Push changes to client
            ///
            void Publish();

            /// @brief Push state changes to client
            ///
            void PublishState();

            /// @brief Save/update entity in database
            ///
            /// @return Successfulness
            bool Save();

            /// @brief Save/update entity state in database
            ///
            /// @return Successfulness
            bool SaveState();

            virtual void JsonGetAttributes(rapidjson::Value& output,
                                           rapidjson::Document::AllocatorType& allocator) const = 0;
            virtual bool JsonSetAttributes(const rapidjson::Value& input) = 0;

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) const;
            bool JsonSet(const rapidjson::Value& input);

            void JsonGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) const;
            bool JsonSetState(const rapidjson::Value& input);
        };
    }
}