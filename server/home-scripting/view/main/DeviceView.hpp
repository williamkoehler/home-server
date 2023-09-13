#pragma once
#include "../../common.hpp"
#include "../View.hpp"

namespace server
{
    namespace scripting
    {
        class Value;

        class DeviceView : public View
        {
            virtual ViewType GetType() const final override;
        };
    }
}