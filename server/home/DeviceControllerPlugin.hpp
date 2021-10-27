#pragma once
#include "common.hpp"
#include "Plugin.hpp"

namespace home
{
	class DeviceControllerCache : public Cache
	{
	protected:
		boost::unordered::unordered_map<uint32_t, rapidjson::CrtValue> propertyList;

	public:
		template<typename Type>
		void SetProperty(uint32_t id, Type value);

		virtual void TakeSnapshot(rapidjson::Document& document) override
		{
			Cache::TakeSnapshot(document);

			rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

			// Add attributes
			rapidjson::Value properties = rapidjson::Value(rapidjson::kArrayType);
			for (std::pair<const uint32_t, rapidjson::CrtValue>& pair : propertyList)
			{
				rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
				property.AddMember("id", rapidjson::Value(pair.first), allocator);
				property.AddMember("value", rapidjson::Value(pair.second, allocator), allocator);
				properties.PushBack(property, allocator);
			}
			document.AddMember("properties", properties, allocator);
		}
	};

	class DeviceControllerPlugin : public Plugin<DeviceControllerCache>
	{
	public:
		friend class server::DeviceController;

		virtual bool Update(size_t cycle) = 0;
	};

	typedef Ref<DeviceControllerPlugin>(CreateDeviceControllerPluginFunction)();
}