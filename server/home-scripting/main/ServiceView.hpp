#pragma once
#include "../View.hpp"
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        class Value;

        class ServiceView : public scripting::View
        {
          public:
            /// @brief Get view type
            ///
            /// @return View  type
            scripting::ViewType GetType() override
            {
                return scripting::ViewType::kServiceViewType;
            }

            /// @brief Get service id
            ///
            /// @return identifier_t Service id
            virtual identifier_t GetID() = 0;

            /// @brief Get service name
            ///
            /// @return std::string Service name
            virtual std::string GetName() = 0;

            /// @brief Set service name
            ///
            /// @param v Service name
            virtual void SetName(const std::string& v) = 0;

            /// @brief Invoke service method
            /// 
            /// @param method Method name
            /// @param parameter Parameter
            virtual void Invoke(const std::string& method, const Value& parameter) = 0;
        };
    }
}