#pragma once
#include "../common.hpp"
#include <home/Home.hpp>

namespace server
{
	class JsonApi;
}

namespace scripting
{
	class ScriptSource;

	// namespace lua
	// {
	// 	class LuaEngine;
	// }

	enum class ScriptLanguage
	{
		kUnknownScriptLanguage = 0,
		kJSScriptLanguage,
	};

	// Get script language enum from name
	// lua -> scriptLanguages::kLuascriptLanguage
	ScriptLanguage ScriptLanguageFromString(std::string lang);

	// Get name from script language enum
	// scriptLanguages::kLuascriptLanguage -> lua
	std::string ScriptLanguageToString(ScriptLanguage language);

	std::string ScriptExtFromScriptLanguage(ScriptLanguage language);

	// Script manager for actions etc...
	// Does not manage plugin scripts
	// > Very important
	class ScriptManager : public boost::enable_shared_from_this<ScriptManager>
	{
	private:
		friend class server::JsonApi;

		boost::shared_mutex mutex;

		boost::atomic<time_t> timestamp = 0;

		// Ref<script::lua::LuaEngine> luaEngine;

		// Unordered map containing all loaded/unloaded scripts
		boost::unordered::unordered_map<uint32_t, Ref<ScriptSource>> scriptSourceList;

		// IO
		void Load();

	public:
		ScriptManager();
		~ScriptManager();
		static Ref<ScriptManager> Create();
		static Ref<ScriptManager> GetInstance();

		//Timestamp
		void UpdateTimestamp();
		inline time_t GetLastTimestamp()
		{
			return timestamp;
		}

		// Script source
		Ref<ScriptSource> AddSource(std::string name, uint32_t sourceID, ScriptLanguage language, uint8_t* data, size_t length);

		virtual inline size_t GetSourceCount()
		{
			boost::shared_lock_guard lock(mutex);
			return scriptSourceList.size();
		}
		virtual Ref<ScriptSource> GetSource(uint32_t sourceID);

		void RemoveSource(uint32_t sourceID);

		//IO
		void Save();
	};
}