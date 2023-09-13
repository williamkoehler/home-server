#pragma once
#include "../common.hpp"
#include <home-common/Worker.hpp>

namespace server
{
    namespace scripting
    {
        class Value;

        enum class ViewType
        {
            kUnknownViewType,
            kHomeViewType,
            kRoomViewType,
            kDeviceViewType,
            kServiceViewType,
        };

        class View : public boost::enable_shared_from_this<View>
        {
          public:
            /// @brief Get view type
            ///
            /// @return View type
            virtual ViewType GetType() const = 0;

            /// @brief Get id
            ///
            /// @return identifier_t ID
            virtual identifier_t GetID() const = 0;

            /// @brief Get name
            ///
            /// @return std::string Name
            virtual std::string GetName() const = 0;

            /// @brief Set name
            ///
            /// @param v Name
            virtual void SetName(const std::string& v) = 0;

            /// @brief Invoke method
            ///
            /// @param method Method name
            /// @param parameter Parameter
            virtual void Invoke(const std::string& method, const Value& parameter) = 0;

            /// @brief Push changes to clients
            ///
            virtual void Publish() = 0;

            /// @brief Push state changes to clients
            ///
            virtual void PublishState() = 0;
        };
    }
}