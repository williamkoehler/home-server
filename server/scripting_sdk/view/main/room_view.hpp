#pragma once
#include "../../common.hpp"
#include "../view.hpp"

namespace server
{
    namespace scripting
    {
        namespace sdk
        {
            class RoomView : public View
            {
                virtual ViewType GetType() const final override
                {
                    return ViewType::kRoomViewType;
                }
            };
        }
    }
}