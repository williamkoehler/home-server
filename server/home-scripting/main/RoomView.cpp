#include "RoomView.hpp"

namespace server
{
    namespace scripting
    {
        ViewType RoomView::GetType() const
        {
            return ViewType::kRoomViewType;
        }

        void RoomView::Invoke(const std::string& method, const Value& parameter)
        {
            (void)method;
            (void)parameter;

            // Do nothing
        }
    }
}