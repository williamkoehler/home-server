//#pragma once
//#include "../../common.hpp"
//#include "../../../homeserver/home/Action.hpp"
//
// class MockDeviceControllerPlugin : public home::DeviceControllerPlugin
//{
// public:
//	PLUGIN_DESCRIPTION(MockDeviceControllerPlugin, "Mock device controller plugin");
//
//	virtual bool Initialize(Ref<home::DeviceController> controller) override { return true; }
//
//	virtual bool Terminate(Ref<home::DeviceController> controller) override { return true; }
//};
//
// Ref<server::DeviceController> MockDeviceController(identifier_t id = 1, const std::string& name = "unknown
// devicecontroller", Ref<home::DeviceControllerPlugin> plugin = nullptr, Ref<server::Room> room = nullptr)
//{
//	if (plugin == nullptr)
//		plugin = boost::make_shared<MockDeviceControllerPlugin>();
//
//	return server::DeviceController::Create(name, id, plugin, room);
//}