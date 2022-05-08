#pragma once
#include "common.hpp"
#include <home-scripting/View.hpp>

namespace server
{
    namespace main
    {
        class Room;

        class RoomView : public scripting::View
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

            virtual Ref<threading::Worker> GetWorker() override;

            std::string GetName();
            void SetName(const std::string& v);
        };
    }
}