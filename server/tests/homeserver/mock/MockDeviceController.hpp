#pragma once
#include "../../../homeserver/home/DeviceController.hpp"
#include "../../common.hpp"

class MockDeviceControllerPlugin : public home::DeviceControllerPlugin
{
  private:
    identifier_t pluginID;

  public:
    MockDeviceControllerPlugin(identifier_t pluginID) : pluginID(pluginID)
    {
    }

    virtual std::string GetPluginName() override
    {
        return "Mock Device Controller Plugin";
    }
    virtual identifier_t GetPluginID() override
    {
        return pluginID;
    }

    virtual bool Initialize(Ref<home::DeviceController> controller) override
    {
        return true;
    }

    virtual bool Terminate(Ref<home::DeviceController> controller) override
    {
        return true;
    }
};

Ref<server::DeviceController> MockDeviceController(identifier_t id = 1,
                                                   const std::string& name = "unknown devicecontroller",
                                                   identifier_t pluginID = 1, Ref<server::Room> room = nullptr)
{
    return server::DeviceController::Create(name, id, boost::make_shared<MockDeviceControllerPlugin>(pluginID), room);
}