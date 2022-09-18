#pragma once
#include "../View.hpp"
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        class RoomView : public View
        {
          public:
            /// @brief Get view type
            ///
            /// @return View  type
            virtual scripting::ViewType GetType() override
            {
                return scripting::ViewType::kRoomViewType;
            }

            /// @brief Get room id
            ///
            /// @return identifier_t Room id
            virtual identifier_t GetID() = 0;

            /// @brief Get room name
            ///
            /// @return std::string Room name
            virtual std::string GetName() = 0;

            /// @brief Set room name
            ///
            /// @param v Room name
            virtual void SetName(const std::string& v) = 0;
        };
    }
}