#pragma once
#include "../common.h"
#include "DraftManager.h"

namespace server
{
	class JsonApi;
}

namespace scripting
{
	class DraftSource : public boost::enable_shared_from_this<DraftSource>
	{
	private:
		friend class server::JsonApi;

		boost::shared_mutex mutex;

		std::string name;
		const uint32_t sourceID;
		const DraftLanguages language;

		uint8_t* data;
		size_t length;

	public:
		DraftSource(std::string name, uint32_t sourceID, DraftLanguages language);
		~DraftSource();
		static Ref<DraftSource> Create(std::string name, uint32_t sourceID, DraftLanguages language);

		void Lock() { mutex.lock_shared(); }
		void Unlock() { mutex.unlock_shared(); }

		std::string GetName()
		{
			boost::shared_lock_guard lock(mutex);
			return name;
		}
		void SetName(std::string v)
		{
			boost::lock_guard lock(mutex);
			name = std::move(v);
		}

		uint32_t GetSourceID() { return sourceID; }

		DraftLanguages GetLanguage() { return language; }

		uint8_t* GetData() { return data; }
		size_t GetLength() { return length; }

		void Update(uint8_t* data, size_t length);

		//IO
		void Save(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator);
	};
}