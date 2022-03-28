#include "Home.hpp"
#include "Device.hpp"
#include "Room.hpp"
#include <home-database/Database.hpp>
#include <home-scripting/Script.hpp>
#include <home-scripting/ScriptManager.hpp>
#include <home-scripting/ScriptSource.hpp>

namespace server
{
    namespace main
    {
        WeakRef<Home> instanceHome;

        Home::Home(Ref<threading::Worker> worker) : worker(std::move(worker))
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

            // Create worker
            Ref<threading::Worker> worker = threading::Worker::Create("Home", 1);
            if (worker == nullptr)
            {
                LOG_ERROR("Failed to create home worker.");
                return nullptr;
            }

            Ref<Home> home = boost::make_shared<Home>(std::move(worker));
            if (home == nullptr)
                return nullptr;

            instanceHome = home;

            try
            {

                // Load from database
                {
                    Ref<Database> database = Database::GetInstance();
                    assert(database != nullptr);

                    // Load rooms
                    if (!database->LoadRooms(boost::bind(&Home::LoadRoom, home, boost::placeholders::_1,
                                                         boost::placeholders::_2, boost::placeholders::_3)))
                    {
                        LOG_ERROR("Loading rooms");
                        return nullptr;
                    }

                    // Load devices
                    if (!database->LoadDevices(boost::bind(&Home::LoadDevice, home, boost::placeholders::_1,
                                                           boost::placeholders::_2, boost::placeholders::_3,
                                                           boost::placeholders::_4, boost::placeholders::_5,
                                                           boost::placeholders::_6)))
                    {
                        LOG_ERROR("Loading devices");
                        return nullptr;
                    }
                }
            }
            catch (std::exception)
            {
                return nullptr;
            }

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
            boost::shared_lock_guard lock(mutex);

            const robin_hood::unordered_node_map<identifier_t, Ref<Room>>::const_iterator it = roomList.find(roomID);
            if (it == roomList.end())
                return nullptr;

            return (*it).second;
        }

        bool Home::RemoveRoom(identifier_t roomID)
        {
            boost::lock_guard lock(mutex);

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
                              identifier_t controllerID, identifier_t roomID, const std::string_view& data)
        {
            // Get script source
            Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
            assert(scriptManager != nullptr);

            Ref<scripting::Script> script = scriptManager->CreateDeviceScript(scriptSourceID);
            if (script == nullptr)
            {
                LOG_ERROR("Create script '{0}'", scriptSourceID);
                return false;
            }

            // Get controller and room
            //! We don't care if no room nor controller is found, since it is allowed to be null
            Ref<Device> controller = GetDevice(controllerID);
            Ref<Room> room = GetRoom(roomID);

            // Create device
            Ref<Device> device = Device::Create(id, name, std::move(script), std::move(controller), std::move(room));

            // Add device
            if (device != nullptr)
            {
                deviceList[device->GetID()] = device;

                // Initialize device
                device->Initialize();

                return true;
            }
            else
                return false;
        }

        Ref<Device> Home::AddDevice(const std::string& name, identifier_t scriptSourceID, identifier_t controllerID,
                                    identifier_t roomID, rapidjson::Value& json)
        {

            // Create script
            Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
            assert(scriptManager != nullptr);

            Ref<scripting::Script> script = scriptManager->CreateDeviceScript(scriptSourceID);
            if (script == nullptr)
            {
                LOG_ERROR("Create script '{0}'", scriptSourceID);
                return nullptr;
            }

            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Reserve room in database
            identifier_t id = database->ReserveDevice();
            if (id == 0)
                return nullptr;

            // Update database
            if (!database->UpdateDevice(id, name, scriptSourceID, controllerID, roomID))
                return nullptr;

            // Get controller and room
            //! We don't care if no room nor controller is found, since it is allowed to be null
            Ref<Device> controller = GetDevice(controllerID);
            Ref<Room> room = GetRoom(roomID);

            // Create new device
            Ref<Device> device = Device::Create(id, name, std::move(script), std::move(controller), std::move(room));

            // Add device
            if (device != nullptr)
            {
                // Lock main mutex
                boost::lock_guard lock(mutex);

                deviceList[device->GetID()] = device;

                // Initialize device
                device->Initialize();
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
            boost::shared_lock_guard lock(mutex);

            const robin_hood::unordered_node_map<identifier_t, Ref<Device>>::const_iterator it = deviceList.find(id);
            if (it == deviceList.end())
                return nullptr;

            return it->second;
        }

        bool Home::RemoveDevice(identifier_t id)
        {
            boost::lock_guard lock(mutex);

            const robin_hood::unordered_node_map<identifier_t, Ref<Device>>::const_iterator it = deviceList.find(id);
            if (it != deviceList.end())
            {
                Ref<Database> database = Database::GetInstance();
                assert(database != nullptr);

                database->RemoveDevice(id);

                // Terminate device
                it->second->Terminate();

                deviceList.erase(it);

                return true;
            }
            else
                return false;
        }

        //! Worker
        void Home::Run()
        {
            // Start worker thread
            worker->Start();
        }

        void Home::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            boost::shared_lock_guard lock(mutex);

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
        }
    }
}