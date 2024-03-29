#pragma once
#include "entity.hpp"
#include "common.hpp"
#include <scripting_sdk/view/main/room_view.hpp>

namespace server
{
    namespace main
    {
        class Device;

        class RoomView;

        class Room : public Entity
        {
          private:
            Ref<RoomView> view;

            // Attributes
            bool hidden;
            int8_t floorNumber;
            std::string roomType;

          public:
            Room(identifier_t id, const std::string& name);
            virtual ~Room();
            static Ref<Room> Create(identifier_t id, const std::string& name);

            virtual EntityType GetType() const override
            {
                return EntityType::kRoomEntityType;
            }

            /// @brief Get room type
            ///
            /// @return Room type
            inline std::string GetRoomType() const
            {
                return roomType;
            }

            /// @brief Set room type
            ///
            /// @param v Room type
            /// @param update Update database flag
            /// @return Successfulness
            void SetRoomType(const std::string& v)
            {
                roomType = v;
            }

            /// @brief Get floor number
            ///
            /// @return Floor number
            inline uint8_t GetFloorNumber() const
            {
                return floorNumber;
            }

            /// @brief Set floor number
            ///
            /// @param v Floor number
            void SetFloorNumber(uint8_t v)
            {
                floorNumber = v;
            }

            /// @brief Get view
            ///
            /// @return Ref<scripting::sdk::View> Room view
            virtual Ref<scripting::sdk::View> GetView() override
            {
                return boost::static_pointer_cast<scripting::sdk::View>(view);
            }

            /// @brief Get room view
            ///
            /// @return Ref<RoomView> Room view
            inline Ref<RoomView> GetRoomView()
            {
                return view;
            }

            virtual void JsonGetAttributes(rapidjson::Value& output,
                                           rapidjson::Document::AllocatorType& allocator) const override;
            virtual bool JsonSetAttributes(const rapidjson::Value& input) override;
        };

        class RoomView final : public scripting::sdk::RoomView
        {
          private:
            WeakRef<Room> room;

          public:
            RoomView(const Ref<Room>& room);
            virtual ~RoomView();

            virtual identifier_t GetID() const override;

            virtual std::string GetName() const override;
            virtual void SetName(const std::string& v) override;

            virtual void Invoke(const std::string& method, const scripting::sdk::Value& parameter) override;

            virtual void Publish() override;
            virtual void PublishState() override;
        };
    }
}