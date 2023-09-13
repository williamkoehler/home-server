#include "HomeView.hpp"
#include "../../ScriptManager.hpp"

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
            return "Home";
        }

        void HomeView::SetName(const std::string& v)
        {
            (void)v;

            // Do nothing
        }

        void HomeView::Invoke(const std::string& method, const Value& parameter)
        {
            (void)method;
            (void)parameter;

            // Do nothing
        }

        void HomeView::Publish()
        {
            // Do nothing
        }

        void HomeView::PublishState()
        {
            // Do nothing
        }
    }
}