#pragma once
#include "../../common.hpp"
#include "../view.hpp"

namespace server
{
    namespace scripting
    {
        namespace sdk
        {
            class RoomView;
            class DeviceView;

            class HomeView : public View
            {
              public:
                // static Ref<HomeView> GetHomeView();

                /// @brief Get view type
                ///
                /// @return ViewType View Type
                virtual ViewType GetType() const final override
                {
                    return ViewType::kHomeViewType;
                }

                virtual identifier_t GetID() const final override
                {
                    return 0;
                }

                virtual std::string GetName() const final override
                {
                    return "Home";
                }

                virtual void SetName(const std::string& v) final override
                {
                    (void)v;

                    // Do nothing
                }

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

                virtual void Invoke(const std::string& method, const Value& parameter) final override
                {
                    (void)method;
                    (void)parameter;

                    // Do nothing
                }
                virtual void Publish() final override
                {
                    // Do nothing
                }
                virtual void PublishState() final override
                {
                    // Do nothing
                }
            };
        }
    }
}