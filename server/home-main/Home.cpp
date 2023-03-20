#include "Home.hpp"
#include "Device.hpp"
#include "Room.hpp"
#include "Service.hpp"
#include <home-database/Database.hpp>
#include <home-scripting/ScriptManager.hpp>

namespace server
{
    namespace main
    {
        WeakRef<Home> instanceHome;

        Home::Home()
        {
        }
        Home::~Home()
        {
            deviceList.clear();
            roomList.clear();
        }
        Ref<Home> Home::Create()
        {
            if (!instanceHome.expired())
                return Ref<Home>(instanceHome);

            Ref<Home> home = boost::make_shared<Home>();
            if (home == nullptr)
                return nullptr;

            instanceHome = home;

            // Load from database
            {
                Ref<Database> database = Database::GetInstance();
                assert(database != nullptr);

                // Load rooms
                if (!database->LoadRooms(boost::bind(&Home::LoadRoom, home, boost::placeholders::_1,
                                                     boost::placeholders::_2, boost::placeholders::_3)))
                {
                    LOG_ERROR("Loading rooms.");
                    return nullptr;
                }

                // Load devices
                if (!database->LoadDevices(boost::bind(&Home::LoadDevice, home, boost::placeholders::_1,
                                                       boost::placeholders::_2, boost::placeholders::_3,
                                                       boost::placeholders::_4, boost::placeholders::_5)))
                {
                    LOG_ERROR("Loading devices.");
                    return nullptr;
                }

                // Load services
                if (!database->LoadServices(boost::bind(&Home::LoadService, home, boost::placeholders::_1,
                                                        boost::placeholders::_2, boost::placeholders::_3,
                                                        boost::placeholders::_4)))
                {
                    LOG_ERROR("Loading services.");
                    return nullptr;
                }
            }

            // Create home view
            home->view = boost::make_shared<HomeView>(home);
            if (home->view == nullptr)
            {
                LOG_ERROR("Create home view.");
                return nullptr;
            }

            // Set home view
            scripting::ScriptManager::SetHomeView(home->view);

            home->UpdateTimestamp();

            return home;
        }
        Ref<Home> Home::GetInstance()
        {
            return Ref<Home>(instanceHome);
        }

        //! Timestamp
        void Home::UpdateTimestamp()
        {
            const time_t ts = time(nullptr);
            timestamp = ts;
        }

        //! Room
        bool Home::LoadRoom(identifier_t id, const std::string& type, const std::string& name)
        {
            // Create new room
            Ref<Room> room = Room::Create(id, type, name);

            // Add room
            if (room != nullptr)
            {
                roomList[room->GetID()] = room;

                return true;
            }
            else
                return false;
        }

        Ref<Room> Home::AddRoom(const std::string& type, const std::string& name, rapidjson::Value& json)
        {
            (void)json;

            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Reserve room in database
            identifier_t id = database->ReserveRoom();
            if (id == 0)
                return nullptr;

            // Update database
            if (!database->UpdateRoom(id, type, name))
                return nullptr;

            // Create new room
            Ref<Room> room = Room::Create(id, type, name);

            // Add room
            if (room != nullptr)
                roomList[room->GetID()] = room;
            else
            {
                database->RemoveRoom(id);
                return nullptr;
            }

            UpdateTimestamp();

            return room;
        }

        Ref<Room> Home::GetRoom(identifier_t roomID)
        {
            // boost::shared_lock_guard lock(mutex);

            const robin_hood::unordered_node_map<identifier_t, Ref<Room>>::const_iterator it = roomList.find(roomID);
            if (it == roomList.end())
                return nullptr;

            return (*it).second;
        }

        bool Home::RemoveRoom(identifier_t roomID)
        {
            if (roomList.erase(roomID))
            {
                Ref<Database> database = Database::GetInstance();
                assert(database != nullptr);

                database->RemoveRoom(roomID);

                return true;
            }
            else
                return false;
        }

        //! Device
        bool Home::LoadDevice(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                              identifier_t roomID, const std::string_view& data)
        {
            (void)data;

            // Get room
            //! We don't care if no room is found, since it is allowed to be null
            Ref<Room> room = GetRoom(roomID);

            // Create device
            Ref<Device> device = Device::Create(id, name, scriptSourceID, room);

            // Add device
            if (device != nullptr)
            {
                deviceList[device->GetID()] = device;

                return true;
            }
            else
                return false;
        }

        Ref<Device> Home::AddDevice(const std::string& name, identifier_t scriptSourceID, identifier_t roomID,
                                    rapidjson::Value& json)
        {
            (void)json;

            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Reserve room in database
            identifier_t id = database->ReserveDevice();
            if (id == 0)
                return nullptr;

            // Update database
            if (!database->UpdateDevice(id, name, scriptSourceID, roomID))
                return nullptr;

            // Get room
            //! We don't care if no room is found, since it is allowed to be null
            Ref<Room> room = GetRoom(roomID);

            // Create new device
            Ref<Device> device = Device::Create(id, name, scriptSourceID, room);

            // Add device
            if (device != nullptr)
            {
                deviceList[device->GetID()] = device;
            }
            else
            {
                database->RemoveDevice(id);
                return nullptr;
            }

            return device;
        }

        Ref<Device> Home::GetDevice(identifier_t id)
        {
            const robin_hood::unordered_node_map<identifier_t, Ref<Device>>::const_iterator it = deviceList.find(id);
            if (it == deviceList.end())
                return nullptr;

            return it->second;
        }

        boost::container::vector<Ref<Device>> Home::FilterDevicesByRoom(const Ref<Room>& room)
        {
            // Reserve device list
            boost::container::vector<Ref<Device>> filteredDeviceList = boost::container::vector<Ref<Device>>();
            filteredDeviceList.reserve(32);

            // Get room id
            identifier_t roomID = room->GetID();

            // Lock home mutex
            // boost::shared_lock_guard lock(mutex);

            for (auto& [id, device] : deviceList)
            {
                if (device->GetRoomID() == roomID)
                    filteredDeviceList.push_back(device);
            }

            return filteredDeviceList;
        }

        boost::container::vector<Ref<Device>> Home::FilterDevicesByScript(
            const Ref<scripting::ScriptSource>& scriptSource)
        {
            // Reserve device list
            boost::container::vector<Ref<Device>> filteredDeviceList = boost::container::vector<Ref<Device>>();
            filteredDeviceList.reserve(32);

            // Get script source id
            identifier_t scriptSourceID = scriptSource->GetID();

            for (auto& [id, device] : deviceList)
            {
                if (device->GetScriptSourceID() == scriptSourceID)
                    filteredDeviceList.push_back(device);
            }

            return filteredDeviceList;
        }

        bool Home::RemoveDevice(identifier_t id)
        {
            const robin_hood::unordered_node_map<identifier_t, Ref<Device>>::const_iterator it = deviceList.find(id);
            if (it != deviceList.end())
            {
                Ref<Database> database = Database::GetInstance();
                assert(database != nullptr);

                database->RemoveDevice(id);

                deviceList.erase(it);

                return true;
            }
            else
                return false;
        }

        //! Service
        bool Home::LoadService(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                               const std::string_view& data)
        {
            (void)data;

            // Create service
            Ref<Service> service = Service::Create(id, name, scriptSourceID);

            // Add service
            if (service != nullptr)
            {
                serviceList[service->GetID()] = service;

                return true;
            }
            else
                return false;
        }

        Ref<Service> Home::AddService(const std::string& name, identifier_t scriptSourceID, rapidjson::Value& json)
        {
            (void)json;

            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Reserve room in database
            identifier_t id = database->ReserveService();
            if (id == 0)
                return nullptr;

            // Update database
            if (!database->UpdateService(id, name, scriptSourceID))
                return nullptr;

            // Create new service
            Ref<Service> service = Service::Create(id, name, scriptSourceID);

            // Add service
            if (service != nullptr)
            {
                serviceList[service->GetID()] = service;
            }
            else
            {
                database->RemoveService(id);
                return nullptr;
            }

            return service;
        }

        Ref<Service> Home::GetService(identifier_t id)
        {
            const robin_hood::unordered_node_map<identifier_t, Ref<Service>>::const_iterator it = serviceList.find(id);
            if (it == serviceList.end())
                return nullptr;

            return it->second;
        }

        bool Home::RemoveService(identifier_t id)
        {
            const robin_hood::unordered_node_map<identifier_t, Ref<Service>>::const_iterator it = serviceList.find(id);
            if (it != serviceList.end())
            {
                Ref<Database> database = Database::GetInstance();
                assert(database != nullptr);

                database->RemoveService(id);

                serviceList.erase(it);

                return true;
            }
            else
                return false;
        }

        void Home::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            output.AddMember("timestamp", rapidjson::Value(timestamp), allocator);

            // Devices
            rapidjson::Value deviceListJson = rapidjson::Value(rapidjson::kArrayType);
            deviceListJson.Reserve(deviceList.size(), allocator);

            for (auto& [id, device] : deviceList)
            {
                assert(device != nullptr);

                rapidjson::Value deviceJson = rapidjson::Value(rapidjson::kObjectType);

                device->JsonGet(deviceJson, allocator);

                deviceListJson.PushBack(deviceJson, allocator);
            }

            output.AddMember("devices", deviceListJson, allocator);

            // Rooms
            rapidjson::Value roomListJson = rapidjson::Value(rapidjson::kArrayType);
            roomListJson.Reserve(roomList.size(), allocator);

            for (auto& [id, room] : roomList)
            {
                assert(room != nullptr);

                rapidjson::Value roomJson = rapidjson::Value(rapidjson::kObjectType);

                room->JsonGet(roomJson, allocator);

                roomListJson.PushBack(roomJson, allocator);
            }

            output.AddMember("rooms", roomListJson, allocator);

            // Services
            rapidjson::Value serviceListJson = rapidjson::Value(rapidjson::kArrayType);
            serviceListJson.Reserve(serviceList.size(), allocator);

            for (auto& [id, service] : serviceList)
            {
                assert(service != nullptr);

                rapidjson::Value serviceJson = rapidjson::Value(rapidjson::kObjectType);

                service->JsonGet(serviceJson, allocator);

                serviceListJson.PushBack(serviceJson, allocator);
            }

            output.AddMember("services", serviceListJson, allocator);
        }

        HomeView::HomeView(const Ref<Home>& home) : home(home)
        {
        }
        HomeView::~HomeView()
        {
        }

        Ref<scripting::RoomView> HomeView::GetRoom(identifier_t id) const
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

        Ref<scripting::DeviceView> HomeView::GetDevice(identifier_t id) const
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