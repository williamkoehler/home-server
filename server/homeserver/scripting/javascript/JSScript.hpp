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

		class JSScript : public Script
		{
		private:
			DuktapeUserData userdata;
			Ref<duk_context> context;

			// Prepare script timeout
			void PrepareTimeout(size_t maxTime = 5);

			// Duktape safe call
			static duk_ret_t PrepareSafe(duk_context* context, void* udata);
			static duk_ret_t ExecuteSafe(duk_context* context, void* udata);

		public:
			JSScript(Ref<ScriptSource> source);
			~JSScript();

			/// @brief Get duktape vm context
			/// @return Duktape context
			inline Ref<duk_context> GetDuktape() { return context; }

			void Execute(const std::string& event);
		};
	}
}