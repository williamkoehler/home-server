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

            virtual identifier_t GetID() const override;

            virtual std::string GetName() const override;
            virtual void SetName(const std::string& v) override;
        };
    }
}