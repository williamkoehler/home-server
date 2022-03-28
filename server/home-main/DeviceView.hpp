#pragma once
#include "common.hpp"
#include <home-scripting/View.hpp>

namespace server
{
    namespace main
    {
        class Device;

        class DeviceView : public scripting::View
        {
          private:
            WeakRef<Device> device;

          public:
            DeviceView(Ref<Device> device);
            virtual ~DeviceView();

            scripting::ViewType GetType() override
            {
                return scripting::ViewType::kDeviceViewType;
            }

            virtual Ref<threading::Worker> GetWorker() override;

            std::string GetName();
            void SetName(const std::string& v);
        };
    }
}