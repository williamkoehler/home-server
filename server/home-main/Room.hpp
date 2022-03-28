#pragma once
#include "common.hpp"

namespace server
{
    namespace main
    {
        class Device;

        class RoomView;

        class Room : public boost::enable_shared_from_this<Room>
        {
          private:
            boost::shared_mutex mutex;

            const identifier_t id;
            std::string type;
            std::string name;

            boost::container::set<identifier_t> deviceList;

            Ref<RoomView> view;

          public:
            Room(identifier_t id, const std::string& type, const std::string& name);
            virtual ~Room();
            static Ref<Room> Create(identifier_t id, const std::string& type, const std::string& name);

            /// @brief Get room id
            /// 
            /// @return Room id 
            inline identifier_t GetID()
            {
                return id;
            }

            /// @brief Get room name
            /// 
            /// @return Room name 
            std::string GetName();

            /// @brief Set room name
            /// 
            /// @param v New room name
            /// @return Successfulness
            bool SetName(const std::string& v);

            /// @brief Get room type
            /// 
            /// @return Room type
            std::string GetType();

            /// @brief Set room type
            /// 
            /// @param v Room type
            /// @return Successfulness
            bool SetType(const std::string& v);

            /// @brief Get room view
            /// 
            /// @return Get room view of this object 
            Ref<RoomView> GetView();

            //! Device

            /// @brief Add device to room
            /// @param device Device
            bool AddDevice(Ref<Device> device);

            /// @brief Get device count
            /// @return Device count
            inline size_t GetDeviceCount()
            {
                boost::shared_lock_guard lock(mutex);
                return deviceList.size();
            }

            /// @brief Remove device from room
            /// @param device Device
            bool RemoveDevice(Ref<Device> device);

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
            void JsonSet(rapidjson::Value& input);
        };
    }
}