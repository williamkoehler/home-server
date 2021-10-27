#pragma once
#include "../common.hpp"

namespace server
{
	class ScriptSource;

	class JsonApi;

	class Script : public boost::enable_shared_from_this<Script>
	{
	private:
		friend class JsonApi;

		std::string name;
		const identifier_t scriptID;

		Ref<ScriptSource> source;

	public:
		Script(const std::string& name, identifier_t scriptID, const Ref<ScriptSource>& source);
		~Script();
		static Ref<Script> Create(const std::string& name, identifier_t scriptID, identifier_t sourceID);
	};
}