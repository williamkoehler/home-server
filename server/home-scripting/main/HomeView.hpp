#pragma once
#include "../View.hpp"
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        class RoomView;
        class DeviceView;

        class HomeView : public scripting::View
        {
          public:
            /// @brief Get view type
            ///
            /// @return View  type
            scripting::ViewType GetType() override
            {
                return scripting::ViewType::kHomeViewType;
            }
            
            /// @brief Get room view
            ///
            /// @param id Room id
            /// @return Ref<RoomView> Room view
            virtual Ref<RoomView> GetRoom(identifier_t id) = 0;

            /// @brief Get device view
            ///
            /// @param id Device id
            /// @return Ref<DeviceView> Device view
            virtual Ref<DeviceView> GetDevice(identifier_t id) = 0;
        };
    }
}