#include "Room.hpp"
#include "Device.hpp"
#include "Home.hpp"
#include <home-database/Database.hpp>

namespace server
{
    namespace main
    {
        Room::Room(identifier_t id, const std::string& name) : Entity(id, name)
        {
        }
        Room::~Room()
        {
        }
        Ref<Room> Room::Create(identifier_t id, const std::string& name)
        {
            Ref<Room> room = boost::make_shared<Room>(id, name);

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

        void Room::JsonGetConfig(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            output.AddMember("roomtype", rapidjson::Value(roomType.data(), roomType.size()), allocator);
        }
        bool Room::JsonSetConfig(const rapidjson::Value& input)
        {
            assert(input.IsObject());

            bool update = false;

            rapidjson::Value::ConstMemberIterator roomTypeIt = input.FindMember("roomtype");
            if (roomTypeIt != input.MemberEnd() && roomTypeIt->value.IsString())
            {
                roomType.assign(roomTypeIt->value.GetString(), roomTypeIt->value.GetStringLength());
                update = true;
            }

            return update;
        }

        //! RoomView
        RoomView::RoomView(const Ref<Room>& room) : room(room)
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