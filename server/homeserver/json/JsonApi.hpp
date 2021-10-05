#pragma once
#include "../common.hpp"
#include "../ExecutionContext.hpp"

namespace home
{
	class DeviceManagerScriptDescription;
	class DeviceManagerScript;
	class DeviceScriptDescription;
	class DeviceScript;
}

namespace scripting
{
	class Script;
	class ScriptSource;
}

namespace server
{
	class Room;
	class Device;
	class DeviceManager;
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

		static void BuildJsonDevice(Ref<Device> device, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		static void DecodeJsonDevice(Ref<Device> device, rapidjson::Value& input);
		static void BuildJsonDeviceState(Ref<Device> device, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		static void DecodeJsonDeviceState(Ref<Device> device, rapidjson::Value& input, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);

		static void BuildJsonDeviceManager(Ref<DeviceManager> deviceManager, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		static void DecodeJsonDeviceManager(Ref<DeviceManager> deviceManager, rapidjson::Value& input);
		static void BuildJsonDeviceManagerState(Ref<DeviceManager> deviceManager, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		static void DecodeJsonDeviceManagerState(Ref<DeviceManager> deviceManager, rapidjson::Value& input, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);

		static void BuildJsonAction(const Ref<Action>& action, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		static void DecodeJsonAction(const Ref<Action>& action, rapidjson::Value& input);
		static void BuildJsonActionState(const Ref<Action>& action, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		static void DecodeJsonActionState(const Ref<Action>& action, rapidjson::Value& input, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);

		// Scripting
		static void BuildJsonScriptSources(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);

		static void BuildJsonScriptSource(const Ref<scripting::ScriptSource>& source, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		static void DecodeJsonScriptSource(const Ref<scripting::ScriptSource>& source, rapidjson::Value& input);

		static void BuildJsonScriptSourceContent(const Ref<scripting::ScriptSource>& source, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		static void DecodeJsonScriptSourceContent(const Ref<scripting::ScriptSource>& source, rapidjson::Value& input);

		static void BuildJsonScript(const Ref<scripting::Script>& script, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		static void BuildJsonScriptState(const Ref<scripting::Script>& script, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		static void DecodeJsonScriptState(const Ref<scripting::Script>& script, rapidjson::Value& input);

		// Plugins
		static void BuildJsonPlugins(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		static void BuildJsonDeviceScript(home::DeviceScriptDescription& script, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
		static void BuildJsonDeviceManagerScript(home::DeviceManagerScriptDescription& script, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);

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
		static void ProcessJsonGetTimestampsMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		// Settings
		static void ProcessJsonGetSettingsMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonSetSettingsMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		// Plugins
		static void ProcessJsonGetPluginsMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		// User
		static void ProcessJsonGetUsersMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonSetUsersMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonSetUserMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		// Home
		static void ProcessJsonGetHomeMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonSetHomeMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		// Room
		static void ProcessJsonAddRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonRemoveRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		static void ProcessJsonGetRoomMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonSetRoomMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		static void ProcessJsonAddDeviceToRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonRemoveDeviceFromRoomMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		// DeviceManager
		static void ProcessJsonAddDeviceManagerMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonRemoveDeviceManagerMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		static void ProcessJsonGetDeviceManagerMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonSetDeviceManagerMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		static void ProcessJsonGetDeviceManagerStateMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonSetDeviceManagerStateMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		static void ProcessJsonGetDeviceManagerStatesMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		// Device
		static void ProcessJsonAddDeviceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonRemoveDeviceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		static void ProcessJsonGetDeviceMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonSetDeviceMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		static void ProcessJsonGetDeviceStateMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonSetDeviceStateMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		static void ProcessJsonGetDeviceStatesMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		// Scripting
		static void ProcessJsonGetScriptSourcesMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		static void ProcessJsonAddScriptSourceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonRemoveScriptSourceMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		static void ProcessJsonGetScriptSourceMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonSetScriptSourceMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		static void ProcessJsonGetScriptSourceContentMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonSetScriptSourceContentMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		// Action
		static void ProcessJsonAddActionMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonRemoveActionMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		static void ProcessJsonGetActionMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonSetActionMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		static void ProcessJsonGetActionStateMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonSetActionStateMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);

		static void ProcessJsonResetActionMessageWS(const Ref<User>& user, rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
		static void ProcessJsonExecuteActionMessageWS(rapidjson::Document& input, rapidjson::Document& output, ExecutionContext& context);
	};
}