#pragma once
#include "common.hpp"
#include <home-common/Worker.hpp>
#include <home-scripting/Script.hpp>
#include <home-scripting/main/HomeView.hpp>

namespace server
{
    namespace main
    {
        class Room;
        class Device;
        class Service;

        class HomeView;

        class RoomView;
        class DeviceView;

        class Home : public boost::enable_shared_from_this<Home>
        {
          private:
            boost::atomic<time_t> timestamp = 0;

            robin_hood::unordered_node_map<identifier_t, Ref<Room>> roomList;
            robin_hood::unordered_node_map<identifier_t, Ref<Device>> deviceList;
            robin_hood::unordered_node_map<identifier_t, Ref<Service>> serviceList;

            Ref<HomeView> view;

            // Database
            bool LoadRoom(identifier_t id, const std::string& type, const std::string& name);
            bool LoadDevice(identifier_t id, const std::string& name, identifier_t scriptSourceID, identifier_t roomID,
                            const std::string_view& data);
            bool LoadService(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                             const std::string_view& data);

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

            //! Room

            /// @brief Add room
            /// @param name Room name
            /// @param type Room type
            /// @param json JSON
            /// @return Room
            Ref<Room> AddRoom(const std::string& type, const std::string& name, rapidjson::Value& json);

            /// @brief Get room count
            /// @return Room count
            inline size_t GetRoomCount()
            {
                // boost::shared_lock_guard lock(mutex);
                return roomList.size();
            }

            /// @brief Get room using room id
            /// @param roomID Room id
            /// @return Room or nullptr
            Ref<Room> GetRoom(identifier_t roomID);

            /// @brief Remove room using room id
            /// @param roomID Room id
            bool RemoveRoom(identifier_t roomID);

            //! Device

            /// @brief Add new device
            ///
            /// @param type Device type
            /// @param name Name
            /// @param backendID Backend id
            /// @param roomID Room id
            /// @param json JSON Data
            /// @return New device
            Ref<Device> AddDevice(const std::string& name, identifier_t backendID, identifier_t roomID,
                                  rapidjson::Value& json);

            /// @brief Get device count
            /// @return Device count
            inline size_t GetDeviceCount()
            {
                // boost::shared_lock_guard lock(mutex);
                return deviceList.size();
            }

            /// @brief Get device using its id
            /// @param id Device id
            /// @return Device or nullptr
            Ref<Device> GetDevice(identifier_t id);

            /// @brief Get devices from a room
            ///
            /// @param room Room (or null to get devices that are not in a room)
            /// @return Filtered devices
            boost::container::vector<Ref<Device>> FilterDevicesByRoom(const Ref<Room>& room);

            /// @brief Get devices that use a specific script
            ///
            /// @param room Room (or null to get devices that are not configured)
            /// @return Filtered devices
            boost::container::vector<Ref<Device>> FilterDevicesByScript(const Ref<scripting::ScriptSource>& scriptSource);

            /// @brief Remove device using its id
            /// @param id Device id
            bool RemoveDevice(identifier_t id);

            //! Service

            /// @brief Add new service
            ///
            /// @param type Service type
            /// @param name Name
            /// @param backendID Backend id
            /// @param json JSON Data
            /// @return New service
            Ref<Service> AddService(const std::string& name, identifier_t backendID, rapidjson::Value& json);

            /// @brief Get service count
            /// @return Service count
            inline size_t GetServiceCount()
            {
                // boost::shared_lock_guard lock(mutex);
                return serviceList.size();
            }

            /// @brief Get service using its id
            /// @param id Service id
            /// @return Service or nullptr
            Ref<Service> GetService(identifier_t id);

            /// @brief Remove service using its id
            /// @param id Service id
            bool RemoveService(identifier_t id);

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
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