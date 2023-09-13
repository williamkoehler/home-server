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

        void Room::JsonGetAttributes(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) const
        {
            assert(output.IsObject());

            output.AddMember("hidden", rapidjson::Value(hidden), allocator);
            output.AddMember("roomtype", rapidjson::Value(roomType.data(), roomType.size(), allocator), allocator);
            output.AddMember("floornumber", rapidjson::Value(floorNumber), allocator);
        }
        bool Room::JsonSetAttributes(const rapidjson::Value& input)
        {
            assert(input.IsObject());

            bool update = false;

            rapidjson::Value::ConstMemberIterator hiddenIt = input.FindMember("hidden");
            if (hiddenIt != input.MemberEnd() && hiddenIt->value.IsBool())
            {
                hidden = hiddenIt->value.GetBool();
                update = true;
            }

            rapidjson::Value::ConstMemberIterator roomTypeIt = input.FindMember("roomtype");
            if (roomTypeIt != input.MemberEnd() && roomTypeIt->value.IsString())
            {
                roomType.assign(roomTypeIt->value.GetString(), roomTypeIt->value.GetStringLength());
                update = true;
            }

            rapidjson::Value::ConstMemberIterator floorNumberIt = input.FindMember("floornumber");
            if (floorNumberIt != input.MemberEnd() && floorNumberIt->value.IsInt())
            {
                floorNumber = floorNumberIt->value.GetInt();
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
            if (Ref<Room> r = room.lock())
                return r->GetID();

            return 0;
        }

        std::string RoomView::GetName() const
        {
            if (Ref<Room> r = room.lock())
                return r->GetName();

            return "";
        }
        void RoomView::SetName(const std::string& v)
        {
            if (Ref<Room> r = room.lock())
                r->SetName(v);
        }

        void RoomView::Invoke(const std::string& method, const scripting::Value& parameter)
        {
            if (Ref<Room> r = room.lock())
                r->Invoke(method, parameter);
        }

        void RoomView::Publish()
        {
            if (Ref<Room> r = room.lock())
                r->Publish();
        }

        void RoomView::PublishState()
        {
            if (Ref<Room> r = room.lock())
                r->PublishState();
        }
    }
}