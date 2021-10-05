#include "JSScript.hpp"

extern "C"
{
	// Interrupt function called by the duktape engine
	// This allows to interruption of any javascript code that runs too long
	duk_ret_t duk_exec_timeout(void* udata)
	{
		scripting::JSUserData* userdata = (scripting::JSUserData*)udata;

		// Test elapsed time
		size_t currentTime = clock();
		size_t elapsedTime = currentTime - userdata->startTime;

		return elapsedTime > userdata->maxTime;
	}
}

namespace scripting
{
	JSScript::JSScript(Ref<ScriptSource> source) : Script(std::move(source)),
		context(nullptr)
	{
	}
	JSScript::~JSScript()
	{
		if (context != nullptr)
			duk_destroy_heap(context);
	}

	bool JSScript::IsReady()
	{
		boost::shared_lock_guard lock(mutex);
		return context != nullptr;
	}

	void JSScript::PrepareTimeout(size_t maxTime)
	{
		userdata.startTime = clock();
		userdata.maxTime = maxTime;
	}

	bool JSScript::Compile()
	{
		assert(source != nullptr);

		boost::lock_guard lock(mutex);

		lastResult = ScriptResult::Success();

		// Destroy old duktape context
		if (context != nullptr)
		{
			duk_destroy_heap(context);
			context = nullptr;
		}

		// Create duktape context
		memset(&userdata, 0, sizeof(JSUserData));
		context = duk_create_heap(nullptr, nullptr, nullptr, &userdata, nullptr);
		if (context == nullptr)
		{
			lastResult = ScriptResult::Failure("(internal) Could not create duktape context.");
			return false;
		}

		// Compile
		if (duk_safe_call(context, JSScript::SafeCompile, source.get(), 0, 0) != 0)
		{
			duk_destroy_heap(context); // Clean
			context = nullptr;

			lastResult = ScriptResult::Failure(duk_safe_to_string(context, -1));
			return false;
		}

		// Call script
		if (duk_safe_call(context, JSScript::SafeCallScript, this, 0, 0) != 0)
		{
			duk_destroy_heap(context); // Clean
			context = nullptr;

			lastResult = ScriptResult::Failure(duk_safe_to_string(context, -1));
			return false;
		}

		return true;
	}

	bool JSScript::Run()
	{
		boost::lock_guard lock(mutex);

		lastResult = ScriptResult::Success();

		if (context != nullptr)
		{
			if (duk_safe_call(context, JSScript::SafeCallRun, this, 0, 0) != 0)
			{
				lastResult = ScriptResult::Failure(duk_safe_to_string(context, -1));
				duk_pop(context); // Pop error
				return false;
			}
		}
		else
		{
			lastResult = ScriptResult::Failure("(internal) Could not run script.");
			return false;
		}

		return true;
	}

	duk_ret_t JSScript::SafeCompile(duk_context* context, void* udata)
	{
		ScriptSource* source = (ScriptSource*)udata;

		// Load modules
		// TODO: Load modules

		// Load script source
		duk_push_lstring(context, (const char*)source->GetData(), source->GetLength());

		// Compile
		duk_compile(context, 0);

		return 0;
	}
	duk_ret_t JSScript::SafeCallScript(duk_context* context, void* udata)
	{
		JSScript* script = (JSScript*)udata;

		// Call script
		duk_call(context, 0);
		duk_pop(context);

		// Verify functions
		duk_push_global_object(context);

		if (!duk_has_prop_lstring(context, -1, "run", 3))
			return duk_error(context, DUK_ERR_ERROR, "Could not find 'function run() { ... }'.");

		duk_pop(context);
	}
	duk_ret_t JSScript::SafeCallRun(duk_context* context, void* udata)
	{
		JSScript* script = (JSScript*)udata;

		// Get argument count
		size_t argCount = duk_get_top(context);

		// Get run function from script
		if (!duk_get_global_lstring(context, "run", 3))
		{
			// Prepare timeout
			script->PrepareTimeout(10);

			// Call run
			duk_call(context, argCount);
		}

		// Pop undefined or return value
		duk_pop(context);

		return 0;
	}
}