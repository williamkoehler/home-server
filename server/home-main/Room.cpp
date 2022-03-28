#include "Room.hpp"
#include "Device.hpp"
#include "Home.hpp"
#include "RoomView.hpp"
#include <home-database/Database.hpp>

namespace server
{
    namespace main
    {
        Room::Room(identifier_t id, const std::string& type, const std::string& name) : id(id), type(type), name(name)
        {
        }
        Room::~Room()
        {
            deviceList.clear();
        }
        Ref<Room> Room::Create(identifier_t id, const std::string& type, const std::string& name)
        {
            Ref<Room> room = boost::make_shared<Room>(id, type, name);

            if (room != nullptr)
            {
                // Initialize view
                room->view = boost::make_shared<RoomView>(room);
                if (room->view == nullptr)
                {
                    LOG_ERROR("Failed to initialize room view.");
                    return nullptr;
                }
            }

            return room;
        }

        std::string Room::GetType()
        {
            boost::shared_lock_guard lock(mutex);
            return type;
        }
        bool Room::SetType(const std::string& v)
        {
            boost::lock_guard lock(mutex);

            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            if (database->UpdateRoomPropType(id, type, v))
            {
                type = v;

                return true;
            }

            return false;
        }

        std::string Room::GetName()
        {
            boost::shared_lock_guard lock(mutex);
            return name;
        }
        bool Room::SetName(const std::string& v)
        {
            boost::lock_guard lock(mutex);

            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            if (database->UpdateRoomPropName(id, name, v))
            {
                name = v;

                return true;
            }

            return false;
        }

        Ref<RoomView> Room::GetView()
        {
            boost::shared_lock_guard lock(mutex);
            return view;
        }

        //! Device
        bool Room::AddDevice(Ref<Device> device)
        {
            assert(device != nullptr);

            boost::lock_guard lock(mutex);

            // Add device id to list
            return deviceList.insert(device->GetID()).second;
        }
        bool Room::RemoveDevice(Ref<Device> device)
        {
            assert(device != nullptr);

            boost::lock_guard lock(mutex);

            // Remove device id from list
            return deviceList.erase(device->GetID());
        }

        void Room::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            // Lock mutex
            boost::shared_lock_guard lock(mutex);

            output.AddMember("id", rapidjson::Value(id), allocator);
            output.AddMember("name", rapidjson::Value(name.c_str(), name.size()), allocator);
            output.AddMember("type", rapidjson::Value(type.data(), type.size()), allocator);

            // Build devices

            // Reserve memory
            rapidjson::Value devicesJson = rapidjson::Value(rapidjson::kArrayType);
            devicesJson.Reserve(deviceList.size(), allocator);

            for (identifier_t& deviceID : deviceList)
                devicesJson.PushBack(rapidjson::Value(deviceID), allocator);

            output.AddMember("devices", devicesJson, allocator);
        }
        void Room::JsonSet(rapidjson::Value& input)
        {
            assert(input.IsObject());

            // Decode properties
            rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
            if (nameIt != input.MemberEnd() && nameIt->value.IsString())
                SetName(std::string(nameIt->value.GetString(), nameIt->value.GetStringLength()));

            rapidjson::Value::MemberIterator typeIt = input.FindMember("type");
            if (typeIt != input.MemberEnd() && typeIt->value.IsUint())
                SetType(std::string(typeIt->value.GetString(), typeIt->value.GetStringLength()));
        }
    }
}