#include "RoomView.hpp"

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