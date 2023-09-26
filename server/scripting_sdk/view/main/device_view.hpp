#pragma once
#include "../../common.hpp"
#include "../view.hpp"

namespace server
{
    namespace scripting
    {
        namespace sdk
        {
            class Value;

            class DeviceView : public View
            {
                virtual ViewType GetType() const final override
                {
                    return ViewType::kDeviceViewType;
                }
            };
        }
    }
}