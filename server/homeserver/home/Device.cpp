#include "Device.h"
#include "../plugin/PluginManager.h"
#include <xxHash/xxhash.h>
#include "DeviceManager.h"
#include "../signal/SignalManager.h"
#include "../json/JsonField.h"

namespace server
{
	Device::Device(std::string name, uint32_t deviceID, Ref<home::DeviceScript> script)
		: roomCount(0), name(std::move(name)), deviceID(deviceID), script(std::move(script))
	{
	}
	Device::~Device()
	{
		script->OnTerminate();
	}
	Ref<Device> Device::Create(std::string name, uint32_t deviceID, uint32_t scriptID, rapidjson::Value& json)
	{
		assert(deviceID != 0 && scriptID != 0);

		Ref<home::DeviceScript> script = PluginManager::GetInstance()->CreateDeviceScript(scriptID);
		if (script == nullptr)
		{
			LOG_ERROR("Create device script '{0}'", scriptID);
			return nullptr;
		}

		Ref<Device> device = boost::make_shared<Device>(std::move(name), deviceID, script);
		if (device == nullptr)
			return nullptr;

		script->device = device;

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

		return device;
	}

	void Device::AddUpdatable()
	{
		Home::GetInstance()->AddDeviceToUpdatables(shared_from_this());
	}

	void Device::Update(Ref<SignalManager> signalManager, size_t cycle)
	{
		boost::lock_guard lock(scriptMutex);

		script->OnUpdate(boost::static_pointer_cast<home::SignalManager>(signalManager), cycle);
	}

	//IO
	void Device::Save(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator)
	{
		assert(json.IsObject());

		boost::shared_lock_guard lock(mutex);

		json.AddMember("name", rapidjson::Value(name.c_str(), name.size(), allocator), allocator);
		json.AddMember("id", rapidjson::Value(deviceID), allocator);
		json.AddMember("script-id", rapidjson::Value(GetScriptID()), allocator);

		rapidjson::Value scriptDataJson = rapidjson::Value(rapidjson::kArrayType);
		server::JsonWriteableFieldCollection collection = server::JsonWriteableFieldCollection(scriptDataJson, allocator);

		script->GetFields(collection);

		json.AddMember("script-data", scriptDataJson, allocator);
	}
}