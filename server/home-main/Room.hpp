#pragma once
#include "Entity.hpp"
#include "common.hpp"
#include <home-scripting/main/RoomView.hpp>

namespace server
{
    namespace main
    {
        class Device;

        class RoomView;

        class Room : public Entity
        {
          private:
            std::string roomType;

            Ref<RoomView> view;

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

            /// @brief Get view
            ///
            /// @return Ref<scripting::View> Room view
            virtual Ref<scripting::View> GetView() override
            {
                return boost::static_pointer_cast<scripting::View>(view);
            }

            /// @brief Get room view
            ///
            /// @return Ref<RoomView> Room view
            inline Ref<RoomView> GetRoomView()
            {
                return view;
            }

            virtual void JsonGetConfig(rapidjson::Value& output,
                                       rapidjson::Document::AllocatorType& allocator) const override;
            virtual bool JsonSetConfig(const rapidjson::Value& input) override;
        };

        class RoomView final : public scripting::RoomView
        {
          private:
            WeakRef<Room> room;

          public:
            RoomView(const Ref<Room>& room);
            virtual ~RoomView();

            virtual identifier_t GetID() const override;

            virtual std::string GetName() const override;
            virtual void SetName(const std::string& v) override;

            virtual void Invoke(const std::string& method, const scripting::Value& parameter) override;

            virtual void Publish() override;
            virtual void PublishState() override;
        };
    }
}