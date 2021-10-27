#pragma once
#include "../../common.hpp"
#include "../ApiContext.hpp"

namespace server
{
	class Room;
	class Device;
	class DeviceController;
	class Action;
	class User;

	class JsonApi
	{
	public:
		// Build

		// Core
		static void BuildJsonSettings(const Ref<User>& user, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);

		// Home
		static void BuildJsonHome(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator, size_t timestamp = 0);
		static void DecodeJsonHome(rapidjson::Value& input);

		static void BuildJsonRoom(Ref<Room> room, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		static void DecodeJsonRoom(Ref<Room> room, rapidjson::Value& input);

		static void BuildJsonDeviceController(Ref<DeviceController> controller, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		static void DecodeJsonDeviceController(Ref<DeviceController> controller, rapidjson::Value& input);
		static void BuildJsonDeviceControllerState(Ref<DeviceController> controller, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		static void DecodeJsonDeviceControllerState(Ref<DeviceController> controller, rapidjson::Value& input, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);

		static void BuildJsonDevice(Ref<Device> device, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		static void DecodeJsonDevice(Ref<Device> device, rapidjson::Value& input);
		static void BuildJsonDeviceState(Ref<Device> device, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		static void DecodeJsonDeviceState(Ref<Device> device, rapidjson::Value& input, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);

		////// Scripting
		////static void BuildJsonScriptSources(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);

		////static void BuildJsonScriptSource(const Ref<scripting::ScriptSource>& source, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		////static void DecodeJsonScriptSource(const Ref<scripting::ScriptSource>& source, rapidjson::Value& input);

		////static void BuildJsonScriptSourceContent(const Ref<scripting::ScriptSource>& source, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		////static void DecodeJsonScriptSourceContent(const Ref<scripting::ScriptSource>& source, rapidjson::Value& input);

		////static void BuildJsonScript(const Ref<scripting::Script>& script, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		////static void BuildJsonScriptState(const Ref<scripting::Script>& script, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		////static void DecodeJsonScriptState(const Ref<scripting::Script>& script, rapidjson::Value& input);

		// User
		static void BuildJsonUsers(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator, size_t timestamp = 0);
		static void DecodeJsonUsers(rapidjson::Value& input);
		static void BuildJsonUser(const Ref<User>& user, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		static void DecodeJsonUser(const Ref<User>& user, rapidjson::Value& input);

	public: // HTTP
		// Build
		static void BuildJsonErrorMessageHTTP(const char* error, rapidjson::Document& output);

		// Process
		static bool ProcessResApiCallHTTP(boost::beast::http::verb verb, std::string_view target, const Ref<User>& user, std::string_view input, rapidjson::StringBuffer& output, std::string& contentType);


	public: // WS
		// Build
		static void BuildJsonAckMessageWS(rapidjson::Document& output);
		static void BuildJsonNAckMessageWS(rapidjson::Document& output);

		// Get timestamps
		static void ProcessJsonGetTimestampsMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);

		// Settings
		static void ProcessJsonGetSettingsMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);
		static void ProcessJsonSetSettingsMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);

		// User
		static void ProcessJsonGetUsersMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);
		static void ProcessJsonSetUsersMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);
		static void ProcessJsonSetUserMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);

		// Home
		static void ProcessJsonGetHomeMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);
		static void ProcessJsonSetHomeMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);

		// Room
		static void ProcessJsonAddRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);
		static void ProcessJsonRemoveRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);

		static void ProcessJsonGetRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);
		static void ProcessJsonSetRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);

		// DeviceController
		static void ProcessJsonAddDeviceControllerMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);
		static void ProcessJsonRemoveDeviceControllerMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);

		static void ProcessJsonGetDeviceControllerMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);
		static void ProcessJsonSetDeviceControllerMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);

		static void ProcessJsonGetDeviceControllerStateMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);
		static void ProcessJsonSetDeviceControllerStateMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);

		static void ProcessJsonGetDeviceControllerStatesMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);

		// Device
		static void ProcessJsonAddDeviceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);
		static void ProcessJsonRemoveDeviceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);

		static void ProcessJsonGetDeviceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);
		static void ProcessJsonSetDeviceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);

		static void ProcessJsonGetDeviceStateMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);
		static void ProcessJsonSetDeviceStateMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);

		static void ProcessJsonGetDeviceStatesMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);

		////// Scripting
		////static void ProcessJsonGetScriptSourcesMessageWS(rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);

		////static void ProcessJsonAddScriptSourceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);
		////static void ProcessJsonRemoveScriptSourceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);

		////static void ProcessJsonGetScriptSourceMessageWS(rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);
		////static void ProcessJsonSetScriptSourceMessageWS(rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);

		////static void ProcessJsonGetScriptSourceContentMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);
		////static void ProcessJsonSetScriptSourceContentMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ApiContext& context);
	};
}