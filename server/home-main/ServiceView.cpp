#include "ServiceView.hpp"
#include "Service.hpp"
#include "Home.hpp"

namespace server
{
    namespace main
    {
        ServiceView::ServiceView(Ref<Service> service) : service(service)
        {
            assert(service != nullptr);
        }
        ServiceView::~ServiceView()
        {
        }

        identifier_t ServiceView::GetID()
        {
            if (Ref<Service> r = service.lock())
                return r->GetID();

            return 0;
        }

        std::string ServiceView::GetName()
        {
            if (Ref<Service> r = service.lock())
                return r->GetName();

            return "";
        }
        void ServiceView::SetName(const std::string& v)
        {
            if (Ref<Service> r = service.lock())
                r->SetName(v);
        }

        void ServiceView::Invoke(const std::string& method, Ref<scripting::Value> parameter)
        {
            if (Ref<Service> r = service.lock())
                r->Invoke(method, parameter);
        }
    }
}