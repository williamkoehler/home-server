#pragma once
#include "common.hpp"
#include <home-scripting/View.hpp>

namespace server
{
    namespace main
    {
        class Home;

        class RoomView;
        class DeviceView;

        class HomeView : public scripting::View
        {
          private:
          public:
            HomeView();
            virtual ~HomeView();

            /// @brief Get view type
            ///
            /// @return View  type
            scripting::ViewType GetType() override
            {
                return scripting::ViewType::kHomeViewType;
            }

            /// @brief Get worker instance
            ///
            /// @return Worker
            virtual Ref<threading::Worker> GetWorker() override;

            /// @brief Get room view
            ///
            /// @param id Room id
            /// @return Room view
            Ref<RoomView> GetRoom(identifier_t id);

            /// @brief Get device view
            ///
            /// @param id Device id
            /// @return Device view
            Ref<DeviceView> GetDevice(identifier_t id);
        };
    }
}