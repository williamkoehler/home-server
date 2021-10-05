#pragma once
#include "../common.hpp"
#include "Result.hpp"
#include "ScriptManager.hpp"
#include "ScriptSource.hpp"
#include <visual/Visual.hpp>
#include <helper/Types.hpp>

namespace server
{
	class JsonApi;
}

namespace scripting
{
	class Field;

	// Share with javascript module
	class JSModule;

	class Script : public boost::enable_shared_from_this<Script>
	{
	protected:
		friend class JSModule;
		friend class server::JsonApi;

		boost::shared_mutex mutex;

		// Source
		Ref<ScriptSource> source;

		// Properties
		boost::unordered_map<identifier_t, Ref<Field>> fieldList;

		ScriptResult lastResult;

		Script(Ref<ScriptSource> source);
		~Script();
	public:
		static Ref<Script> Create(uint32_t scriptSourceID);

		inline ScriptLanguage GetLanguage()
		{
			boost::shared_lock_guard lock(mutex);
			return source->GetLanguage();
		}

		/// @brief Get script source
		/// @return Script source
		inline Ref<ScriptSource> GetSource()
		{
			boost::shared_lock_guard lock(mutex);
			return source;
		}

		/// @brief Check if the current script is ready to run
		/// If not the script needs to be compiled first
		/// @return Readiness
		virtual bool IsReady() = 0;

		/// @brief Compile script source code
		/// @return Successfulness
		virtual bool Compile() = 0;

		/// @brief Call script run method
		/// @return Successfulness
		virtual bool Run() = 0;

		/// @brief Get the last compilation or exection result
		/// @return Compilation or rxecution result
		inline ScriptResult GetLastResult() 
		{
			boost::shared_lock_guard lock(mutex);
			return lastResult;
		}
	};
}