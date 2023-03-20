#include "Device.hpp"
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
        Device::Device(identifier_t id, const std::string& name, const Ref<Room>& room)
            : id(id), name(name), script(nullptr), room(room)
        {
        }
        Device::~Device()
        {
        }
        Ref<Device> Device::Create(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                                   const Ref<Room>& room)
        {
            Ref<Device> device = boost::make_shared<Device>(id, name, room);

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

                    // Initialize script
                    device->script->Initialize();
                }
            }

            return device;
        }

        std::string Device::GetName()
        {
            return name;
        }
        bool Device::SetName(const std::string& v)
        {
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

                    // Initialize script
                    script->Initialize();

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
            if (script != nullptr)
                return script->GetSourceID();

            return 0;
        }

        Ref<Room> Device::GetRoom()
        {
            return room.lock();
        }
        bool Device::SetRoom(const Ref<Room>& v)
        {
            Ref<Room> oldRoomRef = room.lock();
            identifier_t oldRoomID = oldRoomRef != nullptr ? oldRoomRef->GetID() : 0;

            identifier_t roomID = v != nullptr ? v->GetID() : 0;

            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            if (database->UpdateDevicePropRoom(id, oldRoomID, roomID))
            {
                room = v;
                return true;
            }

            return false;
        }

        identifier_t Device::GetRoomID()
        {
            Ref<Room> roomRef = room.lock();
            return roomRef != nullptr ? roomRef->GetID() : 0;
        }

        Ref<DeviceView> Device::GetView()
        {
            return view;
        }

        void Device::Invoke(const std::string& id, const scripting::Value& parameter)
        {
            if (script != nullptr)
                script->PostInvoke(id, parameter);
        }

        void Device::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            // Lock
            // boost::shared_lock_guard lock(mutex);

            output.MemberReserve(6, allocator);

            // Build properties
            output.AddMember("id", rapidjson::Value(id), allocator);

            output.AddMember("name", rapidjson::Value(name.data(), name.size(), allocator), allocator);

            output.AddMember("scriptsourceid",
                             script != nullptr ? rapidjson::Value(script->GetSourceID())
                                               : rapidjson::Value(rapidjson::kNullType),
                             allocator);

            Ref<Room> roomRef = room.lock();
            output.AddMember("roomid",
                             roomRef != nullptr ? rapidjson::Value(roomRef->GetID())
                                                : rapidjson::Value(rapidjson::kNullType),
                             allocator);

            // Script
            rapidjson::Value scriptJson = rapidjson::Value(rapidjson::kObjectType);

            if (script != nullptr)
                script->JsonGet(scriptJson, allocator);
            else
                scriptJson.SetNull();

            output.AddMember("script", scriptJson, allocator);
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

            rapidjson::Value::MemberIterator scriptSourceIDIt = input.FindMember("scriptsourceid");
            if (scriptSourceIDIt != input.MemberEnd() && scriptSourceIDIt->value.IsUint())
                SetScriptSourceID(scriptSourceIDIt->value.GetUint());

            rapidjson::Value::MemberIterator roomIDIt = input.FindMember("roomid");
            if (roomIDIt != input.MemberEnd() && roomIDIt->value.IsUint())
                SetRoom(home->GetRoom(roomIDIt->value.GetUint()));
        }

        void Device::JsonGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            // Lock mutex
            // boost::shared_lock_guard lock(mutex);

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

            if (script != nullptr)
                script->JsonSetState(input);
        }

        DeviceView::DeviceView(const Ref<Device>& device) : device(device)
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