#pragma once
#include "../../common.hpp"
#include "../Script.hpp"
#include "duktape.h"

extern "C"
{
	duk_ret_t duk_exec_timeout(void* udata);
}

namespace server
{
	class ScriptSource;

	namespace javascript
	{
		class JSScript : public Script
		{
		private:
			size_t startTime;
			size_t maxTime;
			Ref<duk_context> context;

			boost::container::vector<Ref<home::Property>> propertyListByID;

			// Prepare script timeout
			void PrepareTimeout(size_t maxTime = 5);

			// Duktape safe call
			static duk_ret_t PrepareSafe(duk_context* context, void* udata);
			static duk_ret_t InvokeSafe(duk_context* context, void* udata);

			static duk_ret_t PropertyGetter(duk_context* context);
			static duk_ret_t PropertySetter(duk_context* context);

		public:
			JSScript(Ref<ScriptSource> source);
			~JSScript();

			/// @brief Get duktape vm context
			/// @return Duktape context
			inline Ref<duk_context> GetDuktape() { return context; }

			inline bool CheckTimeout()
			{
				// Test elapsed time
				size_t currentTime = clock() / (CLOCKS_PER_SEC / 1000);
				size_t elapsedTime = currentTime - startTime;

				return elapsedTime > maxTime;
			}

			virtual bool Invoke(const std::string& event) override;
		};
	}
}