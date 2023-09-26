#pragma once
#include "common.hpp"
#include "entity.hpp"
#include <scripting/script.hpp>
#include <scripting_sdk/view/main/device_view.hpp>

namespace server
{
    namespace main
    {
        class Room;

        class DeviceView;

        class Device final : public Entity
        {
          protected:
            Ref<DeviceView> view;

            // Attributes
            bool hidden;
            WeakRef<Room> room;

          public:
            Device(identifier_t id, const std::string& name);
            virtual ~Device();
            static Ref<Device> Create(identifier_t id, const std::string& name);

            virtual EntityType GetType() const override
            {
                return EntityType::kDeviceEntityType;
            }

            /// @brief Get room
            ///
            /// @return Ref<Room> Room (can be null)
            Ref<Room> GetRoom();

            /// @brief Set room
            ///
            /// @param v Room (can be null)
            void SetRoom(const Ref<Room>& v);

            /// @brief Get room id
            ///
            /// @return identifier_t Room id or zero
            identifier_t GetRoomID();

            /// @brief Get view
            ///
            /// @return Ref<scripting::sdk::View> Device view
            virtual Ref<scripting::sdk::View> GetView() override
            {
                return boost::static_pointer_cast<scripting::sdk::View>(view);
            }

            /// @brief Get device view
            ///
            /// @return Ref<DeviceView> Device view
            inline Ref<DeviceView> GetDeviceView()
            {
                return view;
            }

            virtual void JsonGetAttributes(rapidjson::Value& output,
                                           rapidjson::Document::AllocatorType& allocator) const override;
            virtual bool JsonSetAttributes(const rapidjson::Value& input) override;
        };

        class DeviceView : public scripting::sdk::DeviceView
        {
          private:
            WeakRef<Device> device;

          public:
            DeviceView(const Ref<Device>& device);
            virtual ~DeviceView();

            virtual identifier_t GetID() const override;

            virtual std::string GetName() const override;
            virtual void SetName(const std::string& v) override;

            virtual void Invoke(const std::string& method, const scripting::sdk::Value& parameter) override;

            virtual void Publish() override;
            virtual void PublishState() override;
        };
    }
}