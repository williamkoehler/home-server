#pragma once
#include "entity.hpp"
#include "common.hpp"
#include <api/message.hpp>
#include <common/worker.hpp>
#include <scripting/script.hpp>
#include <scripting/view/main/home_view.hpp>

namespace server
{
    namespace main
    {
        class Entity;
        class Room;
        class Device;
        class Service;

        class HomeView;

        class RoomView;
        class DeviceView;
        class ServiceView;

        class Home : public boost::enable_shared_from_this<Home>
        {
          private:
            boost::atomic<time_t> timestamp = 0;

            robin_hood::unordered_node_map<identifier_t, Ref<Entity>> entityMap;

            Ref<HomeView> view;

            // Database
            bool LoadEntity(identifier_t id, const std::string& type, const std::string& name,
                            identifier_t scriptSourceId, const std::string_view& config, const std::string_view& state);

          public:
            Home();
            virtual ~Home();
            static Ref<Home> Create();
            static Ref<Home> GetInstance();

            //! Timestamp

            /// @brief Update timestamp
            void UpdateTimestamp();

            /// @brief Get timestamp
            /// @return Timestamp
            inline time_t GetLastTimestamp()
            {
                return timestamp;
            }

            /// @brief Add entity
            ///
            /// @param type Entity type
            /// @param name Entity name
            /// @param scriptSourceId Script source id
            /// @param attributesJson Entity attributes
            /// @return Ref<Entity> Entity
            Ref<Entity> AddEntity(EntityType type, const std::string& name, identifier_t scriptSourceId,
                                  const rapidjson::Value& attributesJson);

            /// @brief Get entity count
            /// @return size_t Entity count
            inline size_t GetEntityCount()
            {
                // boost::shared_lock_guard lock(mutex);
                return entityMap.size();
            }

            /// @brief Get entity
            ///
            /// @param entityId Entity id
            /// @return Ref<Entity> Entity or null
            Ref<Entity> GetEntity(identifier_t entityId);

            /// @brief Get room entity
            ///
            /// @param entityId Entity id
            /// @return Ref<Room> Room or null
            Ref<Room> GetRoom(identifier_t entityId);

            /// @brief Get device entity
            ///
            /// @param entityId Entity id
            /// @return Ref<Device> Device or null
            Ref<Device> GetDevice(identifier_t entityId);

            /// @brief Get service entity
            ///
            /// @param entityId Entity id
            /// @return Ref<Service> Service or null
            Ref<Service> GetService(identifier_t entityId);

            /// @brief Remove room using room id
            ///
            /// @param roomID Room id
            bool RemoveEntity(identifier_t entityId);

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) const;

            //! WebSocket Api
            static void WebSocketProcessGetHomeMessage(const Ref<api::User>& user,
                                                       const api::ApiRequestMessage& request,
                                                       api::ApiResponseMessage& response,
                                                       const Ref<api::WebSocketSession>& session);
            static void WebSocketProcessAddEntityMessage(const Ref<api::User>& user,
                                                         const api::ApiRequestMessage& request,
                                                         api::ApiResponseMessage& response,
                                                         const Ref<api::WebSocketSession>& session);
            static void WebSocketProcessRemoveEntityMessage(const Ref<api::User>& user,
                                                            const api::ApiRequestMessage& request,
                                                            api::ApiResponseMessage& response,
                                                            const Ref<api::WebSocketSession>& session);
            static void WebSocketProcessGetEntityMessage(const Ref<api::User>& user,
                                                         const api::ApiRequestMessage& request,
                                                         api::ApiResponseMessage& response,
                                                         const Ref<api::WebSocketSession>& session);
            static void WebSocketProcessSetEntityMessage(const Ref<api::User>& user,
                                                         const api::ApiRequestMessage& request,
                                                         api::ApiResponseMessage& response,
                                                         const Ref<api::WebSocketSession>& session);
            static void WebSocketProcessInvokeDeviceMethodMessage(const Ref<api::User>& user,
                                                                  const api::ApiRequestMessage& request,
                                                                  api::ApiResponseMessage& response,
                                                                  const Ref<api::WebSocketSession>& session);
            static void WebSocketProcessSubscribeToEntityStateMessage(const Ref<api::User>& user,
                                                                      const api::ApiRequestMessage& request,
                                                                      api::ApiResponseMessage& response,
                                                                      const Ref<api::WebSocketSession>& session);
            static void WebSocketProcessUnsubscribeFromEntityStateMessage(const Ref<api::User>& user,
                                                                          const api::ApiRequestMessage& request,
                                                                          api::ApiResponseMessage& response,
                                                                          const Ref<api::WebSocketSession>& session);
            static void WebSocketProcessGetEntityStateMessage(const Ref<api::User>& user,
                                                              const api::ApiRequestMessage& request,
                                                              api::ApiResponseMessage& response,
                                                              const Ref<api::WebSocketSession>& session);
            static void WebSocketProcessSetEntityStateMessage(const Ref<api::User>& user,
                                                              const api::ApiRequestMessage& request,
                                                              api::ApiResponseMessage& response,
                                                              const Ref<api::WebSocketSession>& session);
        };

        class HomeView : public scripting::HomeView
        {
          private:
            WeakRef<Home> home;

          public:
            HomeView(const Ref<Home>& home);
            virtual ~HomeView();

            /// @brief Get room view
            ///
            /// @param id Room id
            /// @return Ref<RoomView> Room view
            virtual Ref<scripting::RoomView> GetRoom(identifier_t id) const override;

            /// @brief Get device view
            ///
            /// @param id Device id
            /// @return Ref<DeviceView> Device view
            virtual Ref<scripting::DeviceView> GetDevice(identifier_t id) const override;
        };
    }
}