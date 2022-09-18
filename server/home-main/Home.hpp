#pragma once
#include "common.hpp"
#include <home-common/Worker.hpp>
#include <home-scripting/Script.hpp>

namespace server
{
    namespace main
    {
        class Room;
        class Device;

        class Home : public boost::enable_shared_from_this<Home>
        {
          private:
            boost::atomic<time_t> timestamp = 0;

            robin_hood::unordered_node_map<identifier_t, Ref<Room>> roomList;
            robin_hood::unordered_node_map<identifier_t, Ref<Device>> deviceList;

            const Ref<Worker> worker;

            // Database
            bool LoadRoom(identifier_t id, const std::string& type, const std::string& name);
            bool LoadDevice(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                            identifier_t controllerID, identifier_t roomID, const std::string_view& data);

          public:
            Home(Ref<Worker> worker);
            virtual ~Home();
            static Ref<Home> Create(Ref<Worker> worker);
            static Ref<Home> GetInstance();

            inline Ref<Worker> GetWorker() const
            {
                return worker;
            }

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
            /// @param controllerID Controller id
            /// @param roomID Room id
            /// @param json JSON Data
            /// @return New device
            Ref<Device> AddDevice(const std::string& name, identifier_t backendID, identifier_t controllerID,
                                  identifier_t roomID, rapidjson::Value& json);

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
            boost::container::vector<Ref<Device>> FilterDevicesByRoom(Ref<Room> room);

            /// @brief Get devices that use a specific script
            ///
            /// @param room Room (or null to get devices that are not configured)
            /// @return Filtered devices
            boost::container::vector<Ref<Device>> FilterDevicesByScript(Ref<scripting::ScriptSource> scriptSource);

            /// @brief Remove device using its id
            /// @param id Device id
            bool RemoveDevice(identifier_t id);

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
        };
    }
}