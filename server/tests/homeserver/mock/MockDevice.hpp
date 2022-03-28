#pragma once
#include "../../../homeserver/home/Device.hpp"
#include "../../common.hpp"

class MockDevicePlugin : public home::DevicePlugin
{
  private:
    identifier_t pluginID;

  public:
    MockDevicePlugin(identifier_t pluginID) : pluginID(pluginID)
    {
    }

    virtual std::string GetPluginName() override
    {
        return "Mock Device Plugin";
    }
    virtual identifier_t GetPluginID() override
    {
        return pluginID;
    }

    virtual bool Initialize(Ref<home::Device> device) override
    {
        return true;
    }

    virtual bool Terminate(Ref<home::Device> device) override
    {
        return true;
    }
};

Ref<server::Device> MockDevice(identifier_t id = 1, const std::string& name = "unknown user", identifier_t pluginID = 1,
                               Ref<server::DeviceController> controller = nullptr, Ref<server::Room> room = nullptr)
{
    return server::Device::Create(name, id, boost::make_shared<MockDevicePlugin>(pluginID), controller, room);
}