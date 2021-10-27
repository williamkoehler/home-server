#include "Script.hpp"
#include "../ScriptSource.hpp"

extern "C"
{
	// Interrupt function called by the duktape engine
	// This allows to interruption of any javascript code that runs too long
	duk_ret_t duk_exec_timeout(void* udata)
	{
		server::javascript::DuktapeUserData* userdata = (server::javascript::DuktapeUserData*)udata;

		// Test elapsed time
		size_t currentTime = clock();
		size_t elapsedTime = currentTime - userdata->startTime;

		return elapsedTime > userdata->maxTime;
	}
}

namespace server
{
	namespace javascript
	{
		Script::Script(const std::string& name, identifier_t scriptID, const Ref<ScriptSource>& source) : server::Script(name, scriptID, source),
			context(nullptr)
		{ }
		Script::~Script()
		{
			if (context != nullptr)
			{
				duk_destroy_heap(context);
				context = nullptr;
			}
		}
		Ref<Script> Script::Create(const std::string& name, identifier_t scriptID, const Ref<ScriptSource>& source)
		{
			assert(source != nullptr);

			Ref<Script> script = boost::make_shared<Script>(name, scriptID, source);

			if (script != nullptr)
			{
				script->context = duk_create_heap(nullptr, nullptr, nullptr, &script->userdata, nullptr);
				if (script->context == nullptr)
				{
					LOG_ERROR("Creating duktape instance");
					return nullptr;
				}
			}

			return script;
		}

		void Script::PrepareTimeout(size_t maxTime)
		{
			userdata.startTime = clock();
			userdata.maxTime = maxTime;
		}

		//bool Script::OnInitialize(Ref<void> parent)
		//{
		//	assert(parent != nullptr);

		//	if (context == nullptr)
		//	{
		//		//TODO Error message
		//		return false;
		//	}

		//	// Import modules

		//	// Compile
		//	if (duk_safe_call(context, JSScript::CompileSafe, this, 0, 0) != 0)
		//	{
		//		//TODO Error message duk_safe_to_string(context, -1);

		//		duk_destroy_heap(context);
		//		context = nullptr;

		//		return false;
		//	}

		//	return true;
		//}

		duk_ret_t Script::CompileSafe(duk_context* context, void* udata)
		{
			Script* script = (Script*)udata;

			// Import modules
			//script->ImportModules();

			// Load script source
			std::string data = script->source->GetData();
			duk_push_lstring(context, (const char*)data.c_str(), data.size());

			// Compile
			duk_compile(context, 0);

			// Call script
			duk_call(context, 0);
			duk_pop(context);

			return DUK_EXEC_SUCCESS;
		}
	}
}