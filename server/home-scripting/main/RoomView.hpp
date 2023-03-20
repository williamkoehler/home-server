#pragma once
#include "../View.hpp"
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        class RoomView : public View
        {
            virtual ViewType GetType() const final override;
        };
    }
}