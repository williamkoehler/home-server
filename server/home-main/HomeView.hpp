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
            WeakRef<Home> home;

          public:
            HomeView(Ref<Home> home);
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
            virtual Ref<Worker> GetWorker() override;

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