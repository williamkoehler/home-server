#include "DeviceView.hpp"
#include "Device.hpp"
#include "Home.hpp"

namespace server
{
    namespace main
    {
        DeviceView::DeviceView(Ref<Device> device) : device(device)
        {
            assert(device != nullptr);
        }
        DeviceView::~DeviceView()
        {
        }

        identifier_t DeviceView::GetID() const
        {
            if (Ref<Device> r = device.lock())
                return r->GetID();

            return 0;
        }

        std::string DeviceView::GetName() const
        {
            if (Ref<Device> r = device.lock())
                return r->GetName();

            return "";
        }
        void DeviceView::SetName(const std::string& v)
        {
            if (Ref<Device> r = device.lock())
                r->SetName(v);
        }

        void DeviceView::Invoke(const std::string& method, const scripting::Value& parameter)
        {
            if (Ref<Device> r = device.lock())
                r->Invoke(method, parameter);
        }
    }
}