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
        HomeView::HomeView()
        {
        }
        HomeView::~HomeView()
        {
        }

        Ref<threading::Worker> HomeView::GetWorker()
        {
            Ref<Home> home = Home::GetInstance();
            assert(home != nullptr);

            return home->GetWorker();
        }

        Ref<RoomView> HomeView::GetRoom(identifier_t id)
        {
            Ref<Home> home = Home::GetInstance();
            assert(home != nullptr);

            // Ger room using identifier
            Ref<Room> room = home->GetRoom(id);
            if (room != nullptr)
                return room->GetView();
            else
                return nullptr;
        }

        Ref<DeviceView> GetDevice(identifier_t id)
        {
            Ref<Home> home = Home::GetInstance();
            assert(home != nullptr);

            // Get device using identifier
            Ref<Device> device = home->GetDevice(id);
            if (device != nullptr)
                return device->GetView();
            else
                return nullptr;
        }
    }
}