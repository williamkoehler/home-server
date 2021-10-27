#pragma once
#include "../common.hpp"
#include "ScriptSource.hpp"

namespace server
{
	class JsonApi;

	class ScriptManager
	{
	private:
		friend class JsonApi;

		boost::shared_mutex mutex;

		boost::unordered::unordered_map<identifier_t, Ref<ScriptSource>> scriptSourceList;

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
		Ref<ScriptSource> AddScriptSource(const std::string& name, identifier_t sourceID, ScriptUsage usage, ScriptLanguage language);

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
	};
}