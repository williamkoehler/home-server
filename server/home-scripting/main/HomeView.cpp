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
    }
}