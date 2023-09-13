#include "device_view.hpp"

namespace server
{
    namespace scripting
    {
        ViewType DeviceView::GetType() const
        {
            return ViewType::kDeviceViewType;
        }
    }
}