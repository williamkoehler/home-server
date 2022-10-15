#pragma once
#include "common.hpp"
#include <home-scripting/main/DeviceView.hpp>

namespace server
{
    namespace main
    {
        class Device;

        class DeviceView : public scripting::DeviceView
        {
          private:
            WeakRef<Device> device;

          public:
            DeviceView(Ref<Device> device);
            virtual ~DeviceView();

            virtual identifier_t GetID() override;

            virtual std::string GetName() override;
            virtual void SetName(const std::string& v) override;

            virtual void Invoke(const std::string& method, const scripting::Value& parameter) override;
        };
    }
}