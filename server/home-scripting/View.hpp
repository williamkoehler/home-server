#pragma once
#include "common.hpp"
#include <home-common/Worker.hpp>

namespace server
{
    namespace scripting
    {
        enum class ViewType
        {
            kUnknownViewType,
            kHomeViewType,
            kRoomViewType,
            kDeviceViewType,
            kServiceViewType,
        };

        class View
        {
          public:
            /// @brief Get view type
            ///
            /// @return View type
            virtual ViewType GetType() = 0;
        };
    }
}