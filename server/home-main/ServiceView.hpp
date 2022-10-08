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

            virtual identifier_t GetID() override;

            virtual std::string GetName() override;
            virtual void SetName(const std::string& v) override;

            virtual void Invoke(const std::string& method, Ref<scripting::Value> parameter) override;
        };
    }
}