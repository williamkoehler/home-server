#pragma once
#include "../common.hpp"
#include "ScriptSource.hpp"

namespace server
{
	class Script;

	class JsonApi;

	class ScriptManager
	{
	private:
		friend class JsonApi;

		boost::shared_mutex mutex;

		boost::unordered::unordered_map<identifier_t, Ref<ScriptSource>> scriptSourceList;

		// Database
		bool LoadScriptSource(identifier_t sourceID, const std::string& name, ScriptUsage usage, ScriptLanguage language, const std::string_view& data);

	public:
		ScriptManager();
		~ScriptManager();
		static Ref<ScriptManager> Create();
		static Ref<ScriptManager> GetInstance();

		//! Script Source

		/// @brief Add script source
		/// @param name Script source name
		/// @param sourceID Script source id
		/// @param json JSON
		/// @return 
		Ref<ScriptSource> AddScriptSource(const std::string& name, ScriptUsage usage, ScriptLanguage language);

		/// @brief Get script source count
		/// @return Script source count
		inline size_t GetScriptSourceCount()
		{
			boost::shared_lock_guard lock(mutex);
			return scriptSourceList.size();
		}

		/// @brief Get device using device id
		/// @param deviceID Device id
		/// @return Device or nullptr
		Ref<ScriptSource> GetScriptSource(identifier_t sourceID);

		/// @brief Remove script source using source id
		/// @param sourceID Script source id
		bool RemoveScriptSource(identifier_t sourceID);

		//! Script
		
		Ref<Script> CreateActionScript(identifier_t sourceID);
	};
}