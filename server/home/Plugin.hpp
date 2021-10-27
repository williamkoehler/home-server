#pragma once
#include "common.hpp"
#include "Value.hpp"

namespace server
{
	class Device;
	class DeviceController;
}

namespace home
{
	typedef bool(EventCallback)(void*);

	class Cache
	{
	protected:
		rapidjson::CrtAllocator allocator;
		boost::unordered::unordered_map<std::string, rapidjson::CrtValue> attributeList;
		boost::unordered::unordered_map<std::string, EventCallback*> eventList;

	public:
		void SetAttribute(std::string id, const char* json)
		{
			rapidjson::CrtDocument attribute;

			attribute.Parse(json);

			// Check for errors
			if (attribute.HasParseError())
			{
				LOG_WARNING("Invalid visual json: {0}\n{1}", rapidjson::GetParseError_En(attribute.GetParseError()), json);
			}
			else
			{
				// Set attribute
				attributeList[id] = rapidjson::CrtValue(attribute, allocator);
			}
		}

		template<typename Callback>
		void SetEvent(std::string id, Callback callback)
		{
			eventList[id] = (EventCallback*)callback;
		}

		virtual void TakeSnapshot(rapidjson::Document& document)
		{
			rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

			// Clear memory
			document.SetNull();
			allocator.Clear();

			// Fill document
			document.SetObject();

			// Add attributes
			rapidjson::Value attributes = rapidjson::Value(rapidjson::kObjectType);
			for (std::pair<const std::string, rapidjson::CrtValue>& pair : attributeList)
				attributes.AddMember(rapidjson::Value(pair.first.data(), pair.first.size(), allocator), rapidjson::Value(pair.second, allocator), allocator);
			document.AddMember("attributes", attributes, allocator);

			// Add events
			rapidjson::Value events = rapidjson::Value(rapidjson::kArrayType);
			for (std::pair<const std::string, EventCallback*>& pair : eventList)
				attributes.PushBack(rapidjson::Value(pair.first.data(), pair.first.size(), allocator), allocator);
			document.AddMember("events", events, allocator);
		}
	};

	template<class CacheType>
	class Plugin
	{
	protected:
		friend class server::Device;

		CacheType cache;

	public:
		virtual std::string GetPluginName() = 0;
		virtual uint32_t GetPluginID() = 0;

		virtual bool Initialize() = 0;

		virtual bool Terminate() = 0;
	};

	#define PLUGIN_DESCRIPTION(class_, name_) \
		static std::string GetPluginName_() { return name_; } \
		static uint32_t GetPluginID_() { return XXH32(name_, strlen(name_), 0x504c5547); } \
		virtual std::string GetPluginName() override { return GetPluginName_(); } \
		virtual uint32_t GetPluginID() override { return GetPluginID_(); }
}