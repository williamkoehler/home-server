#pragma once
#include "../common.hpp"
#include "ScriptSource.hpp"
#include "Scriptable.hpp"
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

		/// @brief Compile script if necessary
		/// @return Successfulness
		virtual bool Prepare(Ref<Scriptable> scriptable) = 0;

		/// @brief Invoke script event
		/// @param event Event name
		/// @return Successfulness
		virtual bool Invoke(const std::string& event) = 0;
	};
}