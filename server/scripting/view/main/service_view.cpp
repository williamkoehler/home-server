#include "service_view.hpp"

namespace server
{
    namespace scripting
    {
        ViewType ServiceView::GetType() const
        {
            return ViewType::kServiceViewType;
        }
    }
}