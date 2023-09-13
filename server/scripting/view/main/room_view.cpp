#include "room_view.hpp"

namespace server
{
    namespace scripting
    {
        ViewType RoomView::GetType() const
        {
            return ViewType::kRoomViewType;
        }
    }
}