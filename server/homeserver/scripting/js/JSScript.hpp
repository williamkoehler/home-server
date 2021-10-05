#pragma once
#include "../../common.hpp"
#include "../Script.hpp"
#include "duktape.h"

namespace scripting
{
	// User data passed to the duktape engine
	struct JSUserData
	{
		size_t startTime;
		size_t maxTime;
	};

	class JSScript : public Script
	{
	private:
		JSUserData userdata;
		duk_context* context;

		static duk_ret_t SafeCompile(duk_context* context, void* udata);
		static duk_ret_t SafeCallScript(duk_context* context, void* udata);
		static duk_ret_t SafeCallRun(duk_context* context, void* udata);

		void PrepareTimeout(size_t maxTime = 5);

	public:
		JSScript(Ref<ScriptSource> source);
		~JSScript();

		virtual bool IsReady() override;

		virtual bool Compile() override;
		virtual bool Run() override;
	};
}