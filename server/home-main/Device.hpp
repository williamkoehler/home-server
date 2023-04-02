#pragma once
#include "common.hpp"
#include <home-scripting/Script.hpp>
#include <home-scripting/main/DeviceView.hpp>

namespace server
{
    namespace main
    {
        class Room;

        class DeviceView;

        class Device : public boost::enable_shared_from_this<Device>
        {
          protected:
            const identifier_t id;
            std::string name;
            Ref<scripting::Script> script;

            WeakRef<Room> room;

            Ref<DeviceView> view;

          public:
            Device(identifier_t id, const std::string& name, const Ref<Room>& room);
            virtual ~Device();
            static Ref<Device> Create(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                                      const Ref<Room>& room);

            /// @brief Get device id
            ///
            /// @return Device id
            inline identifier_t GetID()
            {
                return id;
            }

            /// @brief Get device name
            ///
            /// @return Device name
            std::string GetName();

            /// @brief Set device name
            ///
            /// @param v New device name
            /// @return Successfulness
            bool SetName(const std::string& v);

            /// @brief Set script source id
            ///
            /// @param scriptSource Script source id
            /// @return Successfulness
            bool SetScriptSourceID(identifier_t scriptSourceID);

            /// @brief Get script source id
            ///
            /// @return Script source id
            identifier_t GetScriptSourceID();

            /// @brief Get room
            /// @return Room (can be null)
            Ref<Room> GetRoom();

            /// @brief Set room
            /// @param v Room (can be null)
            /// @return Successfulness
            bool SetRoom(const Ref<Room>& v);

            /// @brief Get room id
            ///
            /// @return identifier_t Room id or zero
            identifier_t GetRoomID();

            /// @brief Get device view
            ///
            /// @return Get device view of this object
            Ref<DeviceView> GetView();

            /// @brief Invoke script method
            ///
            /// @param event Method name
            void Invoke(const std::string& method, const scripting::Value& parameter);

            /// @brief Invoke script update method
            ///
            void Update();

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
            void JsonSet(rapidjson::Value& input);

            void JsonGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
            void JsonSetState(rapidjson::Value& input);
        };

        class DeviceView : public scripting::DeviceView
        {
          private:
            WeakRef<Device> device;

          public:
            DeviceView(const Ref<Device>& device);
            virtual ~DeviceView();

            virtual identifier_t GetID() const override;

            virtual std::string GetName() const override;
            virtual void SetName(const std::string& v) override;

            virtual void Invoke(const std::string& method, const scripting::Value& parameter) override;
        };
    }
}