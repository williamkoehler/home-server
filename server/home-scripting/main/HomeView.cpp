#include "HomeView.hpp"
#include "../ScriptManager.hpp"

namespace server
{
    namespace scripting
    {
        Ref<HomeView> HomeView::GetHomeView()
        {
            return ScriptManager::GetHomeView();
        }

        ViewType HomeView::GetType() const
        {
            return ViewType::kHomeViewType;
        }

        identifier_t HomeView::GetID() const
        {
            return 0;
        }

        std::string HomeView::GetName() const
        {
            return std::string();
        }

        void HomeView::SetName(const std::string& v)
        {
        }
    }
}