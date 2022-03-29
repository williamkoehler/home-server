#include "Device.hpp"
#include "DeviceView.hpp"
#include "Home.hpp"
#include "Room.hpp"
#include <home-database/Database.hpp>
#include <home-scripting/Script.hpp>
#include <home-scripting/ScriptManager.hpp>
#include <home-scripting/ScriptSource.hpp>

namespace server
{
    namespace main
    {
        Device::Device(identifier_t id, const std::string& name, Ref<Device> controller, Ref<Room> room)
            : id(id), name(name), script(nullptr), controller(std::move(controller)), room(std::move(room))
        {
        }
        Device::~Device()
        {
        }
        Ref<Device> Device::Create(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                                   Ref<Device> controller, Ref<Room> room)
        {
            Ref<Device> device = boost::make_shared<Device>(id, name, std::move(controller), std::move(room));

            if (device != nullptr)
            {
                // Initialize view
                device->view = boost::make_shared<DeviceView>(device);
                if (device->view == nullptr)
                {
                    LOG_ERROR("Failed to initialize device view.");
                    return nullptr;
                }

                if (scriptSourceID != 0)
                {
                    // Create script
                    Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
                    assert(scriptManager != nullptr);

                    device->script = scriptManager->CreateDeviceScript(scriptSourceID, device->view);
                    if (device->script == nullptr)
                    {
                        LOG_ERROR("Create device script '{0}'", scriptSourceID);
                        return nullptr;
                    }
                }
            }

            return device;
        }

        std::string Device::GetName()
        {
            boost::lock_guard lock(mutex);
            return name;
        }
        bool Device::SetName(const std::string& v)
        {
            boost::lock_guard lock(mutex);

            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            if (database->UpdateDevicePropName(id, name, v))
            {
                name = v;

                return true;
            }

            return false;
        }

        bool Device::SetScriptSourceID(identifier_t scriptSourceID)
        {
            boost::lock_guard lock(mutex);

            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            if (database->UpdateDevicePropScriptSource(id, scriptSourceID))
            {
                if (scriptSourceID != 0)
                {
                    // Create script
                    Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
                    assert(scriptManager != nullptr);

                    script = scriptManager->CreateDeviceScript(scriptSourceID, view);
                    if (script == nullptr)
                    {
                        LOG_ERROR("Create device script '{0}'", scriptSourceID);
                        return false;
                    }

                    return true;
                }
                else
                {
                    script = nullptr;
                    return true;
                }
            }

            return false;
        }
        identifier_t Device::GetScriptSourceID()
        {
            boost::lock_guard lock(mutex);

            if (script != nullptr)
                return script->GetSourceID();

            return 0;
        }

        Ref<Device> Device::GetController()
        {
            boost::shared_lock_guard lock(mutex);
            return controller;
        }
        bool Device::SetController(Ref<Device> v)
        {
            boost::lock_guard lock(mutex);

            identifier_t oldControllerID = controller != nullptr ? controller->GetID() : 0;
            identifier_t newControllerID = v != nullptr ? v->GetID() : 0;

            // Cannot assign itself as controller
            // This would lead to an infinite loop of death ;-)
            if (id == newControllerID)
                return false;

            // A controller cannot be controlled
            // There cannot be a chain of controllers
            if (v != nullptr)
            {
                if (v->GetController() != nullptr)
                    return false;
            }

            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            if (database->UpdateDevicePropController(id, oldControllerID, newControllerID))
            {
                controller = v;
                return true;
            }

            return false;
        }

        Ref<Room> Device::GetRoom()
        {
            boost::shared_lock_guard lock(mutex);
            return room;
        }
        bool Device::SetRoom(Ref<Room> v)
        {
            boost::lock_guard lock(mutex);

            identifier_t oldRoomID = room != nullptr ? room->GetID() : 0;
            identifier_t newRoomID = v != nullptr ? v->GetID() : 0;

            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            if (database->UpdateDevicePropRoom(id, oldRoomID, newRoomID))
            {
                room = v;
                return true;
            }

            return false;
        }

        Ref<DeviceView> Device::GetView()
        {
            boost::shared_lock_guard lock(mutex);
            return view;
        }

        void Device::Initialize()
        {
            script->Initialize();
        }

        void Device::Invoke(const std::string& event)
        {
            script->Invoke(event);
        }

        void Device::Terminate()
        {
            script->Terminate();
        }

        void Device::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            // Lock
            boost::shared_lock_guard lock(mutex);

            output.MemberReserve(6, allocator);

            // Build properties
            output.AddMember("id", rapidjson::Value(id), allocator);

            output.AddMember("name", rapidjson::Value(name.data(), name.size(), allocator), allocator);

            output.AddMember("scriptsourceid",
                             script != nullptr ? rapidjson::Value(script->GetSourceID())
                                               : rapidjson::Value(rapidjson::kNullType),
                             allocator);

            output.AddMember("controllerid",
                             controller != nullptr ? rapidjson::Value(controller->id)
                                                   : rapidjson::Value(rapidjson::kNullType),
                             allocator);

            output.AddMember("roomid", room != nullptr ? rapidjson::Value(id) : rapidjson::Value(rapidjson::kNullType),
                             allocator);

            // Attributes
            rapidjson::Value attributesJson = rapidjson::Value(rapidjson::kObjectType);
            script->JsonGet(attributesJson, allocator);

            output.AddMember("attributes", attributesJson, allocator);
        }
        void Device::JsonSet(rapidjson::Value& input)
        {
            assert(input.IsObject());

            Ref<Home> home = Home::GetInstance();
            assert(home != nullptr);

            // Decode properties
            rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
            if (nameIt != input.MemberEnd() && nameIt->value.IsString())
                SetName(std::string(nameIt->value.GetString(), nameIt->value.GetStringLength()));

            rapidjson::Value::MemberIterator controllerIDIt = input.FindMember("controllerid");
            if (controllerIDIt != input.MemberEnd() && controllerIDIt->value.IsUint())
                SetController(home->GetDevice(controllerIDIt->value.GetUint()));

            rapidjson::Value::MemberIterator roomIDIt = input.FindMember("roomid");
            if (roomIDIt != input.MemberEnd() && roomIDIt->value.IsUint())
                SetRoom(home->GetRoom(roomIDIt->value.GetUint()));
        }

        void Device::JsonGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            // Lock mutex
            boost::shared_lock_guard lock(mutex);

            // Build properties
            output.AddMember("id", rapidjson::Value(id), allocator);

            rapidjson::Value state = rapidjson::Value(rapidjson::kObjectType);

            if (script != nullptr)
                script->JsonGetState(state, allocator);

            output.AddMember("state", state, allocator);
        }
        void Device::JsonSetState(rapidjson::Value& input)
        {
            assert(input.IsObject());

            script->JsonSetState(input);
        }
    }
}