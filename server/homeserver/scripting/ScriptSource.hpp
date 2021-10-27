#pragma once
#include "../common.hpp"

namespace server
{
	class JsonApi;

	enum class ScriptUsage
	{
		KUnknownUsage,
		kForActionUsage,
	};

	enum class ScriptLanguage
	{
		kUnknownScriptLanguage,
		kNativeScriptLanguage,
		kJSScriptLanguage
	};

	class ScriptSource : public boost::enable_shared_from_this<ScriptSource>
	{
	private:
		friend class server::JsonApi;

		boost::shared_mutex mutex;

		std::string name;
		const identifier_t sourceID;

		const ScriptUsage usage;
		const ScriptLanguage language;
		std::string data;

	public:
		ScriptSource(const std::string& name, identifier_t sourceID, ScriptUsage usage, ScriptLanguage language);
		~ScriptSource();
		static Ref<ScriptSource> Create(const std::string& name, identifier_t sourceID, ScriptUsage usage, ScriptLanguage language);

		inline std::string GetName()
		{
			boost::shared_lock_guard lock(mutex);
			return name;
		}
		inline void SetName(const std::string& v)
		{
			boost::lock_guard lock(mutex);
			name = v;
		}

		inline identifier_t GetSourceID() { return sourceID; }

		inline ScriptUsage GetUsage() { return usage; }

		inline ScriptLanguage GetLanguage() { return language; };

		inline std::string GetData()
		{
			boost::shared_lock_guard lock(mutex);
			return data;
		}
		inline void SetData(const std::string& v)
		{
			boost::lock_guard lock(mutex);
			data = v;
		}
	};
}