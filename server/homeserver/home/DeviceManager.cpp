#include "Device.hpp"
#include "../plugin/PluginManager.hpp"
#include <xxHash/xxhash.h>
#include "DeviceManager.hpp"
#include "../signal/SignalManager.hpp"
#include "../json/JsonField.hpp"

namespace server
{
	DeviceManager::DeviceManager(std::string name, uint32_t managerID, Ref<home::DeviceManagerScript> script)
		: name(std::move(name)), managerID(managerID), script(std::move(script))
	{
	}
	DeviceManager::~DeviceManager()
	{
		script->OnTerminate();
	}
	Ref<DeviceManager> DeviceManager::Create(std::string name, uint32_t managerID, uint32_t scriptID, rapidjson::Value& json)
	{
		Ref<home::DeviceManagerScript> script = PluginManager::GetInstance()->CreateDeviceManagerScript(scriptID);
		if (script == nullptr)
		{
			LOG_ERROR("Failing to create device manager script '{0}'.", scriptID);
			return nullptr;
		}

		Ref<DeviceManager> deviceManager = boost::make_shared<DeviceManager>(std::move(name), managerID, script);
		if (deviceManager == nullptr)
			return nullptr;

		script->deviceManager = deviceManager;

		rapidjson::Value::MemberIterator scriptDataIt = json.FindMember("script-data");
		if (scriptDataIt != json.MemberEnd() && scriptDataIt->value.IsArray())
		{
			for (rapidjson::Value::ValueIterator it = scriptDataIt->value.Begin(); it != scriptDataIt->value.End(); it++)
			{
				if (it->IsObject())
				{
					rapidjson::Value::MemberIterator idIt = it->FindMember("id");
					rapidjson::Value::MemberIterator valueIt = it->FindMember("value");

					if (idIt != it->MemberEnd() && idIt->value.IsUint64() &&
						valueIt != it->MemberEnd())
					{
						server::JsonReadableField field = server::JsonReadableField(
							idIt->value.GetUint64(),
							valueIt->value);
						script->SetField(field);
					}
				}
			}
		}

		if (!script->OnInitialize())
			return nullptr;

		return deviceManager;
	}
	
	void DeviceManager::AddUpdatable()
	{
		Home::GetInstance()->AddDeviceManagerToUpdatables(shared_from_this());
	}

	void DeviceManager::Update(Ref<SignalManager> signalManager, size_t cycle)
	{
		boost::lock_guard lock(scriptMutex);

		script->OnUpdate(boost::static_pointer_cast<home::SignalManager>(signalManager), cycle);
	}

	//IO
	void DeviceManager::Save(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator)
	{
		assert(json.IsObject());

		boost::shared_lock_guard lock(mutex);

		json.AddMember("name", rapidjson::Value(name.c_str(), name.size()), allocator);
		json.AddMember("id", rapidjson::Value(managerID), allocator);
		json.AddMember("script-id", rapidjson::Value(GetScriptID()), allocator);

		rapidjson::Value scriptDataJson = rapidjson::Value(rapidjson::kArrayType);
		server::JsonWriteableFieldCollection collection = server::JsonWriteableFieldCollection(scriptDataJson, allocator);

		script->GetFields(collection);

		json.AddMember("script-data", scriptDataJson, allocator);
	}
}