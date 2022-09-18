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

        Ref<Worker> DeviceView::GetWorker()
        {
            Ref<Home> home = Home::GetInstance();
            assert(home != nullptr);

            return home->GetWorker();
        }

        identifier_t DeviceView::GetID()
        {
            Ref<Device> r = device.lock();

            if (r != nullptr)
                return r->GetID();

            return 0;
        }

        std::string DeviceView::GetName()
        {
            Ref<Device> r = device.lock();

            if (r != nullptr)
                return r->GetName();

            return "";
        }
        void DeviceView::SetName(const std::string& v)
        {
            Ref<Device> r = device.lock();

            if (r != nullptr)
                r->SetName(v);
        }
    }
}