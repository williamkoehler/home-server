#pragma once
#include "common.hpp"
#include <home-threading/Worker.hpp>

namespace server
{
    namespace scripting
    {
        enum class ViewType
        {
            kUnknownViewType,
            kDeviceViewType,
            kRoomViewType,
        };

        class View
        {
          public:
            virtual ViewType GetType() = 0;

            virtual Ref<threading::Worker> GetWorker() = 0;
        };
    }
}