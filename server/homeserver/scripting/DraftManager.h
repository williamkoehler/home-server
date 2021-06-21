#pragma once
#include "common.h"
#include <home/Home.h>

namespace server
{
	class JsonApi;
}

namespace scripting
{
	class DraftSource;

	// namespace lua
	// {
	// 	class LuaEngine;
	// }

	enum class DraftLanguages
	{
		kUnknownDraftLanguage = 0,
		//kLuaDraftLanguage,
	};

	// Get script language enum from name
	// lua -> DraftLanguages::kLuaDraftLanguage
	DraftLanguages DraftLanguageFromString(std::string lang);

	// Get name from script language enum
	// DraftLanguages::kLuaDraftLanguage -> lua
	std::string DraftLanguageToString(DraftLanguages language);

	std::string DraftExtFromDraftLanguage(DraftLanguages language);

	// Draft manager for actions etc...
	// Does not manage plugin scripts
	class DraftManager : public boost::enable_shared_from_this<DraftManager>
	{
	private:
		friend class server::JsonApi;

		boost::shared_mutex mutex;

		boost::atomic<time_t> timestamp = 0;

		// Ref<scripting::lua::LuaEngine> luaEngine;

		// Unordered map containing all loaded/unloaded drafts
		boost::unordered::unordered_map<uint32_t, Ref<DraftSource>> draftSourceList;

		// IO
		void Load();

	public:
		DraftManager();
		~DraftManager();
		static Ref<DraftManager> Create();
		static Ref<DraftManager> GetInstance();

		//Timestamp
		void UpdateTimestamp();
		inline time_t GetLastTimestamp()
		{
			return timestamp;
		}

		// Draft source
		Ref<DraftSource> AddSource(std::string name, uint32_t sourceID, DraftLanguages language, uint8_t* data, size_t length);

		virtual inline size_t GetSourceCount()
		{
			boost::shared_lock_guard lock(mutex);
			return draftSourceList.size();
		}
		virtual Ref<DraftSource> GetSource(uint32_t sourceID);

		void RemoveSource(uint32_t sourceID);

		//IO
		void Save();
	};
}