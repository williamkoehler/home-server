#include "JSScript.hpp"
#include "../ScriptSource.hpp"

extern "C"
{
	// Interrupt function called by the duktape engine
	// This allows to interruption of any javascript code that runs too long
	duk_ret_t duk_exec_timeout(void* udata)
	{
		server::javascript::DuktapeUserData* userdata = (server::javascript::DuktapeUserData*)udata;

		// Test elapsed time
		size_t currentTime = clock() / (CLOCKS_PER_SEC / 1000);
		size_t elapsedTime = currentTime - userdata->startTime;

		return elapsedTime > userdata->maxTime;
	}
}

namespace server
{
	namespace javascript
	{
		JSScript::JSScript(Ref<ScriptSource> source) : Script(source),
			context(nullptr)
		{ }
		JSScript::~JSScript()
		{
		}

		void JSScript::PrepareTimeout(size_t maxTime)
		{
			userdata.startTime = clock() / (CLOCKS_PER_SEC / 1000);
			userdata.maxTime = maxTime;
		}

		duk_ret_t JSScript::PrepareSafe(duk_context* context, void* udata)
		{
			JSScript* script = (JSScript*)udata;

			// Setup default variables
			duk_push_object(context);
			duk_put_global_lstring(context, "events", 6);

			// Import modules
			//script->ImportModules();

			// Load script source
			std::string data = script->source->GetData();
			duk_push_lstring(context, (const char*)data.c_str(), data.size());

			std::string name = script->source->GetName();
			duk_push_lstring(context, (const char*)name.c_str(), name.size());

			// Compile
			duk_compile(context, 0);

			// Call script
			duk_call(context, 0);
			duk_pop(context);

			return DUK_EXEC_SUCCESS;
		}

		duk_ret_t JSScript::ExecuteSafe(duk_context* context, void* udata)
		{
			std::string_view* event = (std::string_view*)udata;

			// Get events object
			if (!duk_get_global_lstring(context, "events", 6))
				return DUK_RET_ERROR;

			// Get event function
			if (!duk_get_prop_lstring(context, -1, (const char*)event->data(), event->size()))
				return DUK_RET_ERROR;

			// Execute event
			duk_call(context, 0);
			duk_pop(context);

			return DUK_EXEC_SUCCESS;
		}

		void JSScript::Execute(const std::string& event)
		{
			// Check if compilation stage is needed
			if (source->GetChecksum() != checksum || context == nullptr)
			{
				context = Ref<duk_context>(duk_create_heap(nullptr, nullptr, nullptr, &userdata, nullptr), [](duk_context* context) -> void { duk_destroy_heap(context); });
				if (context == nullptr)
					return;

				// Prepare context
				if (duk_safe_call(context.get(), JSScript::PrepareSafe, this, 0, 0) != 0)
				{
					//TODO Error message duk_safe_to_string(context, -1);

					context = nullptr;
					return;
				}
			}

			// Execute
			PrepareTimeout(5000); // 5 seconds

			std::string_view e = std::string_view(event.data(), event.size());
			if (duk_safe_call(context.get(), JSScript::ExecuteSafe, &e, 0, 0) != 0)
			{
				//TODO Error message duk_safe_to_string(context, -1);

				context = nullptr;
				return;
			}
		}
	}
}