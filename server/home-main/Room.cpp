#include "Room.hpp"
#include "Device.hpp"
#include "Home.hpp"
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
            return type;
        }
        bool Room::SetType(const std::string& v)
        {
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
            return name;
        }
        bool Room::SetName(const std::string& v)
        {
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
            return view;
        }

        void Room::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            output.AddMember("id", rapidjson::Value(id), allocator);
            output.AddMember("name", rapidjson::Value(name.c_str(), name.size()), allocator);
            output.AddMember("type", rapidjson::Value(type.data(), type.size()), allocator);
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

        RoomView::RoomView(Ref<Room> room) : room(std::move(room))
        {
            assert(room != nullptr);
        }
        RoomView::~RoomView()
        {
        }

        identifier_t RoomView::GetID() const
        {
            Ref<Room> r = room.lock();

            if (r != nullptr)
                return r->GetID();

            return 0;
        }

        std::string RoomView::GetName() const
        {
            Ref<Room> r = room.lock();

            if (r != nullptr)
                return r->GetName();

            return "";
        }
        void RoomView::SetName(const std::string& v)
        {
            Ref<Room> r = room.lock();

            if (r != nullptr)
                r->SetName(v);
        }
    }
}