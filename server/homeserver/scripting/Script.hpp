#pragma once
#include "../common.hpp"
#include "ScriptSource.hpp"

namespace server
{
	class JsonApi;

	class Script : public boost::enable_shared_from_this<Script>
	{
	protected:
		friend class JsonApi;

		Ref<ScriptSource> source;
		uint64_t checksum;

	public:
		Script(Ref<ScriptSource> source);
		~Script();

		inline identifier_t GetSourceID() { return source->GetSourceID(); }

		virtual void Execute(const std::string& event) = 0;
	};
}