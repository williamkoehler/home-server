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
        };

        class View
        {
          public:
            /// @brief Get view type
            ///
            /// @return View type
            virtual ViewType GetType() = 0;

            /// @brief Get view worker
            /// Note: Should always be valid (not null)
            ///
            /// @return View worker
            virtual Ref<Worker> GetWorker() = 0;
        };
    }
}