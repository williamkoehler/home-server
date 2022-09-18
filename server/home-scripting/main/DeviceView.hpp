#pragma once
#include "../View.hpp"
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        class DeviceView : public scripting::View
        {
          public:
            /// @brief Get view type
            ///
            /// @return View  type
            scripting::ViewType GetType() override
            {
                return scripting::ViewType::kDeviceViewType;
            }

            /// @brief Get device id
            ///
            /// @return identifier_t Device id
            virtual identifier_t GetID() = 0;

            /// @brief Get device name
            ///
            /// @return std::string Device name
            virtual std::string GetName() = 0;

            /// @brief Set device name
            ///
            /// @param v Device name
            virtual void SetName(const std::string& v) = 0;
        };
    }
}