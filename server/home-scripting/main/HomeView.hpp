#pragma once
#include "../View.hpp"
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        class RoomView;
        class DeviceView;

        class HomeView : public View
        {
          public:
            static Ref<HomeView> GetHomeView();

            virtual ViewType GetType() const final override;

            virtual identifier_t GetID() const final override;

            virtual std::string GetName() const final override;

            virtual void SetName(const std::string& v) final override;

            /// @brief Get room view
            ///
            /// @param id Room id
            /// @return Ref<RoomView> Room view
            virtual Ref<RoomView> GetRoom(identifier_t id) const = 0;

            /// @brief Get device view
            ///
            /// @param id Device id
            /// @return Ref<DeviceView> Device view
            virtual Ref<DeviceView> GetDevice(identifier_t id) const = 0;
        };
    }
}