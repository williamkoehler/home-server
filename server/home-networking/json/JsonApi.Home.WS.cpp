#include "JsonApi.hpp"
#include <home-main/Device.hpp>
#include <home-main/Room.hpp>

namespace server
{
    namespace networking
    {
        //! Home
        void JsonApi::ProcessJsonGetHomeMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                  rapidjson::Document& output, ApiContext& context)
        {
            assert(input.IsObject() && output.IsObject());

            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            home->JsonGet(output, allocator);
        }

        //! Room
        void JsonApi::ProcessJsonAddRoomMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                  rapidjson::Document& output, ApiContext& context)
        {
            assert(user != nullptr);
            assert(input.IsObject() && output.IsObject());

            if (user->GetAccessLevel() < users::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                context.Error(ApiError::kError_AccessLevelToLow);
                return;
            }

            // Process request
            rapidjson::Value::MemberIterator typeIt = input.FindMember("type");
            rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
            if (typeIt == input.MemberEnd() || !typeIt->value.IsString() || nameIt == input.MemberEnd() ||
                !nameIt->value.IsString())
            {
                context.Error("Missing type and/or name");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);
            Ref<main::Room> room =
                home->AddRoom(std::string(typeIt->value.GetString(), typeIt->value.GetStringLength()),
                              std::string(nameIt->value.GetString(), nameIt->value.GetStringLength()), json);
            if (room == nullptr)
            {
                //! Error failed to add room
                context.Error(ApiError::kError_InternalError);
                return;
            }

            room->JsonGet(output, allocator);
        }
        void JsonApi::ProcessJsonRemoveRoomMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                     rapidjson::Document& output, ApiContext& context)
        {
            assert(user != nullptr);
            assert(input.IsObject() && output.IsObject());

            if (user->GetAccessLevel() < users::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                context.Error(ApiError::kError_AccessLevelToLow);
                return;
            }

            // Process request
            rapidjson::Value::MemberIterator roomIDIt = input.FindMember("id");
            if (roomIDIt == input.MemberEnd() || !roomIDIt->value.IsUint())
            {
                context.Error("Missing id");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            if (!home->RemoveRoom(roomIDIt->value.GetUint()))
            {
                //! Error failed to remove room
                context.Error(ApiError::kError_InternalError);
                return;
            }
        }

        void JsonApi::ProcessJsonGetRoomMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                  rapidjson::Document& output, ApiContext& context)
        {
            assert(user != nullptr);
            assert(input.IsObject() && output.IsObject());

            // Process request
            rapidjson::Value::MemberIterator roomIDIt = input.FindMember("id");
            if (roomIDIt == input.MemberEnd() || !roomIDIt->value.IsUint())
            {
                context.Error("Missing id");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            // Get room
            Ref<main::Room> room = home->GetRoom(roomIDIt->value.GetUint());
            if (room == nullptr)
            {
                context.Error(ApiError::kError_InvalidIdentifier);
                return;
            }

            room->JsonGet(output, allocator);
        }
        void JsonApi::ProcessJsonSetRoomMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                  rapidjson::Document& output, ApiContext& context)
        {
            assert(input.IsObject() && output.IsObject());

            // Process request
            rapidjson::Value::MemberIterator roomIDIt = input.FindMember("id");
            if (roomIDIt == input.MemberEnd() || !roomIDIt->value.IsUint())
            {
                context.Error("Missing id");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            // Get device
            Ref<main::Room> room = home->GetRoom(roomIDIt->value.GetUint());
            if (room == nullptr)
            {
                context.Error(ApiError::kError_InvalidIdentifier);
                return;
            }

            room->JsonSet(input);
            room->JsonGet(output, allocator);
        }

        //! Device
        void JsonApi::ProcessJsonAddDeviceMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                    rapidjson::Document& output, ApiContext& context)
        {
            assert(user != nullptr);
            assert(input.IsObject() && output.IsObject());

            if (user->GetAccessLevel() < users::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                context.Error(ApiError::kError_AccessLevelToLow);
                return;
            }

            // Process request
            rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
            rapidjson::Value::MemberIterator scriptSourceIDIt = input.FindMember("scriptsourceid");
            rapidjson::Value::MemberIterator controllerIDIt = input.FindMember("controllerid");
            rapidjson::Value::MemberIterator roomIDIt = input.FindMember("roomid");
            if (!nameIt->value.IsString() || scriptSourceIDIt == input.MemberEnd() ||
                !scriptSourceIDIt->value.IsUint() || controllerIDIt == input.MemberEnd() ||
                (!controllerIDIt->value.IsUint() && !controllerIDIt->value.IsNull()) || roomIDIt == input.MemberEnd() ||
                (!roomIDIt->value.IsUint() && !roomIDIt->value.IsNull()))
            {
                context.Error("Missing name, scriptsourceid, controllerid, and/or roomid");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            // Add new device
            rapidjson::Value json = rapidjson::Value(rapidjson::kObjectType);
            Ref<main::Device> device =
                home->AddDevice(nameIt->value.GetString(), scriptSourceIDIt->value.GetUint(),
                                controllerIDIt->value.IsUint() ? controllerIDIt->value.GetUint() : 0,
                                roomIDIt->value.IsUint() ? roomIDIt->value.GetUint() : 0, json);
            if (device == nullptr)
            {
                //! Error failed to add device
                context.Error(ApiError::kError_InternalError);
                return;
            }

            device->JsonGet(output, allocator);
        }
        void JsonApi::ProcessJsonRemoveDeviceMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                       rapidjson::Document& output, ApiContext& context)
        {
            assert(user != nullptr);
            assert(input.IsObject() && output.IsObject());

            if (user->GetAccessLevel() < users::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                context.Error(ApiError::kError_AccessLevelToLow);
                return;
            }

            // Process request
            rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
            if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
            {
                context.Error("Missing id");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            // Remove device
            if (!home->RemoveDevice(deviceIDIt->value.GetUint()))
            {
                //! Error failed to remove device
                context.Error(ApiError::kError_InternalError);
                return;
            }
        }

        void JsonApi::ProcessJsonGetDeviceMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                    rapidjson::Document& output, ApiContext& context)
        {
            assert(input.IsObject() && output.IsObject());

            // Process request
            rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
            if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
            {
                context.Error("Missing id");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            // Get device
            Ref<main::Device> device = home->GetDevice(deviceIDIt->value.GetUint());
            if (device == nullptr)
            {
                context.Error(ApiError::kError_InvalidIdentifier);
                return;
            }

            device->JsonGet(output, allocator);
        }
        void JsonApi::ProcessJsonSetDeviceMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                    rapidjson::Document& output, ApiContext& context)
        {
            assert(input.IsObject() && output.IsObject());

            // Process request
            rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
            if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
            {
                context.Error("Missing id");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            // Get device
            Ref<main::Device> device = home->GetDevice(deviceIDIt->value.GetUint());
            if (device == nullptr)
            {
                context.Error(ApiError::kError_InvalidIdentifier);
                return;
            }

            device->JsonSet(input);
            device->JsonGet(output, allocator);
        }

        void JsonApi::ProcessJsonInvokeDeviceEventMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                            rapidjson::Document& output, ApiContext& context)
        {
            assert(input.IsObject() && output.IsObject());

            // Process request
            rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
            rapidjson::Value::MemberIterator eventIt = input.FindMember("event");
            if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint() || eventIt == input.MemberEnd() ||
                !eventIt->value.IsString())
            {
                context.Error("Missing id and/or event");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            // Get device
            Ref<main::Device> device = home->GetDevice(deviceIDIt->value.GetUint());
            if (device == nullptr)
            {
                context.Error(ApiError::kError_InvalidIdentifier);
                return;
            }

            // Invoke event
            device->Invoke(std::string(eventIt->value.GetString(), eventIt->value.GetStringLength()));
        }

        void JsonApi::ProcessJsonGetDeviceStateMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                         rapidjson::Document& output, ApiContext& context)
        {
            assert(input.IsObject() && output.IsObject());

            // Process request
            rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
            if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
            {
                context.Error("Missing id");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            // Get device
            Ref<main::Device> device = home->GetDevice(deviceIDIt->value.GetUint());
            if (device == nullptr)
            {
                context.Error(ApiError::kError_InvalidIdentifier);
                return;
            }

            device->JsonGetState(output, allocator);
        }
        void JsonApi::ProcessJsonSetDeviceStateMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                         rapidjson::Document& output, ApiContext& context)
        {
            assert(input.IsObject() && output.IsObject());

            // Process request
            rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
            if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
            {
                context.Error("Missing id");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            // Get device
            Ref<main::Device> device = home->GetDevice(deviceIDIt->value.GetUint());
            if (device == nullptr)
            {
                context.Error(ApiError::kError_InvalidIdentifier);
                return;
            }

            device->JsonSetState(input);
            device->JsonGetState(output, allocator);
        }
    }
}