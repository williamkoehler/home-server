#include "JsonApi.h"
#include "../Core.h"

#include "../plugin/PluginManager.h"
#include <Plugin.h>
#include <Script.h>
#include <home/DeviceScript.h>
#include <home/DeviceManagerScript.h>

namespace server
{
	// Plugins
	void JsonApi::BuildJsonPlugins(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(output.IsObject());

		Ref<PluginManager> plugin = PluginManager::GetInstance();
		assert(plugin != nullptr);

		boost::shared_lock_guard lock(plugin->mutex);

		output.AddMember("timestamp", rapidjson::Value(plugin->timestamp), allocator);

		// DeviceScripts
		rapidjson::Value deviceScriptListJson = rapidjson::Value(rapidjson::kArrayType);

		boost::unordered::unordered_map<uint32_t, home::DeviceScriptDescription>& deviceScriptList = plugin->deviceScriptList;
		for (std::pair<uint32_t, home::DeviceScriptDescription> item : deviceScriptList)
		{
			rapidjson::Value deviceScriptJson = rapidjson::Value(rapidjson::kObjectType);

			BuildJsonDeviceScript(item.second, deviceScriptJson, allocator);

			deviceScriptListJson.PushBack(deviceScriptJson, allocator);
		}

		output.AddMember("devicescripts", deviceScriptListJson, allocator);

		// DeviceManagerScripts
		rapidjson::Value deviceManagerScriptListJson = rapidjson::Value(rapidjson::kArrayType);

		boost::unordered::unordered_map<uint32_t, home::DeviceManagerScriptDescription>& deviceManagerScriptList = plugin->deviceManagerScriptList;
		for (std::pair<uint32_t, home::DeviceManagerScriptDescription> item : deviceManagerScriptList)
		{
			rapidjson::Value deviceManagerScriptJson = rapidjson::Value(rapidjson::kObjectType);

			BuildJsonDeviceManagerScript(item.second, deviceManagerScriptJson, allocator);

			deviceManagerScriptListJson.PushBack(deviceManagerScriptJson, allocator);
		}

		output.AddMember("devicemanagerscripts", deviceManagerScriptListJson, allocator);
	}

	void JsonApi::BuildJsonDeviceScript(home::DeviceScriptDescription& script, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(output.IsObject());

		output.AddMember("name", rapidjson::Value(script.name.c_str(), script.name.size(), allocator), allocator);
		output.AddMember("type", rapidjson::Value(script.type), allocator);
		output.AddMember("scriptid", rapidjson::Value(script.scriptID), allocator);

		// Visuals
		rapidjson::Value propertiesJson = rapidjson::Value(rapidjson::kArrayType);

		boost::container::vector<Ref<home::Visual>>& propertyList = script.propertyList;

		// Reserve memory
		propertiesJson.Reserve(propertyList.size(), allocator);

		for (Ref<home::Visual> visual : propertyList)
		{
			rapidjson::Value visualJson = rapidjson::Value(rapidjson::kObjectType);

			visual->GetRepresentation(visualJson, allocator);

			propertiesJson.PushBack(visualJson, allocator);
		}

		output.AddMember("properties", propertiesJson, allocator);

		rapidjson::Value parametersJson = rapidjson::Value(rapidjson::kArrayType);

		boost::container::vector<Ref<home::Visual>>& parameterList = script.parameterList;

		// Reserve memory
		parametersJson.Reserve(parameterList.size(), allocator);

		for (Ref<home::Visual> visual : parameterList)
		{
			rapidjson::Value visualJson = rapidjson::Value(rapidjson::kObjectType);

			visual->GetRepresentation(visualJson, allocator);

			parametersJson.PushBack(visualJson, allocator);
		}

		output.AddMember("parameters", parametersJson, allocator);
	}

	void JsonApi::BuildJsonDeviceManagerScript(home::DeviceManagerScriptDescription& script, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(output.IsObject());

		output.AddMember("name", rapidjson::Value(script.name.c_str(), script.name.size(), allocator), allocator);
		output.AddMember("scriptid", rapidjson::Value(script.scriptID), allocator);

		// Visuals
		rapidjson::Value visualsJson = rapidjson::Value(rapidjson::kObjectType);

		// Parameter visuals
		rapidjson::Value propertiesJson = rapidjson::Value(rapidjson::kArrayType);

		boost::container::vector<Ref<home::Visual>>& propertyList = script.propertyList;

		// Reserve memory
		propertiesJson.Reserve(propertyList.size(), allocator);

		for (Ref<home::Visual> visual : propertyList)
		{
			rapidjson::Value visualJson = rapidjson::Value(rapidjson::kObjectType);

			visual->GetRepresentation(visualJson, allocator);

			propertiesJson.PushBack(visualJson, allocator);
		}

		output.AddMember("properties", propertiesJson, allocator);
	}
}