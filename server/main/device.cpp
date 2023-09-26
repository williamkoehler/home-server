#include "device.hpp"
#include "home.hpp"
#include "room.hpp"
#include <database/database.hpp>

namespace server
{
    namespace main
    {
        Device::Device(identifier_t id, const std::string& name) : Entity(id, name)
        {
        }
        Device::~Device()
        {
        }
        Ref<Device> Device::Create(identifier_t id, const std::string& name)
        {
            Ref<Device> device = boost::make_shared<Device>(id, name);

            if (device != nullptr)
            {
                // Initialize view
                device->view = boost::make_shared<DeviceView>(device);
                if (device->view == nullptr)
                {
                    LOG_ERROR("Failed to initialize device view.");
                    return nullptr;
                }
            }

            return device;
        }

        Ref<Room> Device::GetRoom()
        {
            return room.lock();
        }
        void Device::SetRoom(const Ref<Room>& v)
        {
            room = v;
        }

        identifier_t Device::GetRoomID()
        {
            Ref<Room> roomRef = room.lock();
            return roomRef != nullptr ? roomRef->GetID() : 0;
        }

        void Device::JsonGetAttributes(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) const
        {
            assert(output.IsObject());

            output.AddMember("hidden", rapidjson::Value(hidden), allocator);
            if (Ref<Room> roomRef = room.lock())
                output.AddMember("roomid", rapidjson::Value(roomRef->GetID()), allocator);
            else
                output.AddMember("roomid", rapidjson::Value(rapidjson::kNullType), allocator);
        }

        bool Device::JsonSetAttributes(const rapidjson::Value& input)
        {
            assert(input.IsObject());

            bool update = false;

            rapidjson::Value::ConstMemberIterator hiddenIt = input.FindMember("hidden");
            if (hiddenIt != input.MemberEnd() && hiddenIt->value.IsBool())
            {
                hidden = hiddenIt->value.GetBool();
                update = true;
            }

            rapidjson::Value::ConstMemberIterator roomIdIt = input.FindMember("roomid");
            if (roomIdIt != input.MemberEnd() && roomIdIt->value.IsUint())
            {
                Ref<Home> home = Home::GetInstance();
                assert(home != nullptr);

                SetRoom(home->GetRoom(roomIdIt->value.GetUint()));
                update = true;
            }

            return update;
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

        void DeviceView::Invoke(const std::string& method, const scripting::sdk::Value& parameter)
        {
            if (Ref<Device> r = device.lock())
                r->Invoke(method, parameter);
        }

        void DeviceView::Publish()
        {
            if (Ref<Device> r = device.lock())
                r->Publish();
        }

        void DeviceView::PublishState()
        {
            if (Ref<Device> r = device.lock())
                r->PublishState();
        }
    }
}