#include "ServiceView.hpp"

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