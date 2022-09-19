#include "RoomView.hpp"
#include "Home.hpp"
#include "Room.hpp"

namespace server
{
    namespace main
    {
        RoomView::RoomView(Ref<Room> room) : room(room)
        {
            assert(room != nullptr);
        }
        RoomView::~RoomView()
        {
        }

        identifier_t RoomView::GetID()
        {
            Ref<Room> r = room.lock();

            if (r != nullptr)
                return r->GetID();

            return 0;
        }

        std::string RoomView::GetName()
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