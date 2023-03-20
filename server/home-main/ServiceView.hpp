#pragma once
#include "common.hpp"
#include <home-scripting/main/ServiceView.hpp>

namespace server
{
    namespace main
    {
        class Service;

        class ServiceView : public scripting::ServiceView
        {
          private:
            WeakRef<Service> service;

          public:
            ServiceView(Ref<Service> service);
            virtual ~ServiceView();

            virtual identifier_t GetID() const override;

            virtual std::string GetName() const override;
            virtual void SetName(const std::string& v) override;

            virtual void Invoke(const std::string& method, const scripting::Value& parameter) override;
        };
    }
}