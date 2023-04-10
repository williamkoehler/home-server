#pragma once
#include "common.hpp"
#include <home-scripting/Script.hpp>
#include <home-scripting/View.hpp>

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

          public:
            Entity(identifier_t id, const std::string& name);
            virtual ~Entity();
            static Ref<Entity> Create(identifier_t id, EntityType type, const std::string& name, identifier_t scriptSourceID,
                               const rapidjson::Value& config, const rapidjson::Value& state);
            static Ref<Entity> Create(identifier_t id, EntityType type, const std::string& name, identifier_t scriptSourceID,
                               const std::string_view& config, const std::string_view& state);

            /// @brief Get entity type
            ///
            /// @return EntityType Entity type
            virtual EntityType GetType() = 0;

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
            identifier_t GetScriptSourceId()
            {
                return script != nullptr ? script->GetSourceID() : 0;
            }

            /// @brief Get view
            ///
            /// @return Ref<scripting::View> Get view of this object
            virtual Ref<scripting::View> GetView() = 0;

            /// @brief Save/update entity data in database
            ///
            /// @return Successfulness
            bool SaveConfig();

            /// @brief Save/update entity configuration in database
            ///
            /// @return Successfulness
            bool SaveState();

            virtual void JsonGetConfig(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) = 0;
            virtual bool JsonSetConfig(const rapidjson::Value& input) = 0;

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
            bool JsonSet(const rapidjson::Value& input);

            /// @brief Invoke script method
            ///
            /// @param event Method name
            void ApiInvoke(const std::string& method, const scripting::Value& parameter);

            void ApiGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator, ApiContext& context);
            bool ApiSet(const rapidjson::Value& input, ApiContext& context);

            void ApiGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator,
                             ApiContext& context);
            bool ApiSetState(const rapidjson::Value& input, ApiContext& context);
        };
    }
}