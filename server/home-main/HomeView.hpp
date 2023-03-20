#pragma once
#include "common.hpp"
#include <home-scripting/main/HomeView.hpp>

namespace server
{
    namespace main
    {
        class Home;

        class RoomView;
        class DeviceView;

        class HomeView : public scripting::HomeView
        {
          private:
            WeakRef<Home> home;

          public:
            HomeView(Ref<Home> home);
            virtual ~HomeView();

             /// @brief Get room view
            ///
            /// @param id Room id
            /// @return Ref<RoomView> Room view
            virtual Ref<scripting::RoomView> GetRoom(identifier_t id) const override;

            /// @brief Get device view
            ///
            /// @param id Device id
            /// @return Ref<DeviceView> Device view
            virtual Ref<scripting::DeviceView> GetDevice(identifier_t id) const override;
        };
    }
}