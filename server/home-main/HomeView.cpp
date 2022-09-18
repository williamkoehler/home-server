#include "HomeView.hpp"
#include "Device.hpp"
#include "DeviceView.hpp"
#include "Home.hpp"
#include "Room.hpp"
#include "RoomView.hpp"

namespace server
{
    namespace main
    {
        HomeView::HomeView(Ref<Home> home) : home(home)
        {
        }
        HomeView::~HomeView()
        {
        }

        Ref<Worker> HomeView::GetWorker()
        {
            Ref<Home> r = home.lock();
            assert(r != nullptr);

            if (r != nullptr)
                return r->GetWorker();

            LOG_FATAL("Home is not initialized.");
        }

        Ref<RoomView> HomeView::GetRoom(identifier_t id)
        {
            Ref<Home> r = home.lock();
            assert(r != nullptr);

            // Get room using identifier
            Ref<Room> room = r->GetRoom(id);
            if (room != nullptr)
                return room->GetView();
            else
                return nullptr;
        }

        Ref<DeviceView> HomeView::GetDevice(identifier_t id)
        {
            Ref<Home> r = home.lock();
            assert(r != nullptr);

            // Get device using identifier
            Ref<Device> device = r->GetDevice(id);
            if (device != nullptr)
                return device->GetView();
            else
                return nullptr;
        }
    }
}