#pragma once
#include "common.hpp"
#include <home-scripting/Script.hpp>

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

            WeakRef<Device> controller;
            WeakRef<Room> room;

            Ref<DeviceView> view;

          public:
            Device(identifier_t id, const std::string& name, Ref<Device> controller, Ref<Room> room);
            virtual ~Device();
            static Ref<Device> Create(identifier_t id, const std::string& name, identifier_t scriptSourceID,
                                      Ref<Device> controller, Ref<Room> room);

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

            /// @brief Get controller
            /// @return Controller (can be null)
            Ref<Device> GetController();

            /// @brief Set controller
            /// @param v Controller (can be null)
            /// @return Successfulness
            bool SetController(Ref<Device> v);

            /// @brief Get controller id
            ///
            /// @return identifier_t Controller id or zero
            identifier_t GetControllerID();

            /// @brief Get room
            /// @return Room (can be null)
            Ref<Room> GetRoom();

            /// @brief Set room
            /// @param v Room (can be null)
            /// @return Successfulness
            bool SetRoom(Ref<Room> v);

            /// @brief Get room id
            ///
            /// @return identifier_t Room id or zero
            identifier_t GetRoomID();

            /// @brief Get device view
            ///
            /// @return Get device view of this object
            Ref<DeviceView> GetView();

            /// @brief Initialize script
            ///
            void Initialize();

            /// @brief Invoke script method
            ///
            /// @param event Method name
            void Invoke(const std::string& method);

            /// @brief Terminate script
            ///
            void Terminate();

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
            void JsonSet(rapidjson::Value& input);

            void JsonGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
            void JsonSetState(rapidjson::Value& input);
        };
    }
}