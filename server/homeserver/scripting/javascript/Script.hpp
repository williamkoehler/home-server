#pragma once
#include "../../common.hpp"
#include "../Script.hpp"
#include "duktape.h"

namespace server
{
	class ScriptSource;

	namespace javascript
	{
		// User data used for script timeout
		struct DuktapeUserData
		{
			size_t startTime;
			size_t maxTime;
		};

		class Script : public server::Script
		{
		private:
			DuktapeUserData userdata;
			duk_context* context;

			Ref<ScriptSource> source;

			// Prepare script timeout
			void PrepareTimeout(size_t maxTime = 5);

			// Duktape safe call
			static duk_ret_t CompileSafe(duk_context* context, void* udata);

		public:
			Script(const std::string& name, identifier_t scriptID, const Ref<ScriptSource>& source);
			~Script();
			static Ref<Script> Create(const std::string& name, identifier_t scriptID, const Ref<ScriptSource>& source);

			/// @brief Get duktape vm context
			/// @return Duktape context
			inline duk_context* GetDuktape() { return context; }
		};
	}
}