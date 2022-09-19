#pragma once
#include "common.hpp"
#include <home-scripting/main/RoomView.hpp>

namespace server
{
    namespace main
    {
        class Room;

        class RoomView : public scripting::RoomView
        {
          private:
            WeakRef<Room> room;

          public:
            RoomView(Ref<Room> room);
            virtual ~RoomView();

            virtual scripting::ViewType GetType() override
            {
                return scripting::ViewType::kRoomViewType;
            }

            virtual identifier_t GetID() override;

            virtual std::string GetName() override;
            virtual void SetName(const std::string& v) override;
        };
    }
}