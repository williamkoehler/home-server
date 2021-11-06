#pragma once
#include "../common.hpp"

namespace server
{
	class Database;

	class JsonApi;

	enum class ScriptUsage
	{
		kUnknownUsage,
		kForActionUsage,
	};

	std::string StringifyScriptUsage(ScriptUsage usage);
	ScriptUsage ParseScriptUsage(const std::string& usage);

	enum class ScriptLanguage
	{
		kUnknownScriptLanguage,
		kNativeScriptLanguage,
		kJSScriptLanguage
	};

	std::string StringifyScriptLanguage(ScriptLanguage language);
	ScriptLanguage ParseScriptLanguage(const std::string& language);

	class ScriptSource : public boost::enable_shared_from_this<ScriptSource>
	{
	private:
		friend class Database;
		friend class JsonApi;

		boost::shared_mutex mutex;

		std::string name;
		const identifier_t sourceID;

		const ScriptUsage usage;
		const ScriptLanguage language;
		std::string data;

		/// @brief Checksum (changes when the data changes)
		boost::atomic_uint64_t checksum;

	public:
		ScriptSource(const std::string& name, identifier_t sourceID, ScriptUsage usage, ScriptLanguage language, const std::string_view& data);
		~ScriptSource();

		std::string GetName();
		bool SetName(const std::string& v);

		inline identifier_t GetSourceID() { return sourceID; }

		inline ScriptUsage GetUsage() { return usage; }

		inline ScriptLanguage GetLanguage() { return language; }

		std::string GetData();
		bool SetData(const std::string_view& v);

		inline uint64_t GetChecksum() { return checksum; }
	};
}