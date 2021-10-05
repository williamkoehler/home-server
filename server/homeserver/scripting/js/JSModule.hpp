#pragma once
#include "../../common.hpp"
#include "JSScript.hpp"

namespace scripting
{
	class JSModule
	{
	private:
		// Script	
		static duk_ret_t ScriptGetScriptID(duk_context* context);

	public:
		static void JSLoadScriptModule(duk_context* context, Ref<JSScript> script);
		static void JSLoadHomeModule(duk_context* context, Ref<JSScript> script);
		static void JSLoadUserModule(duk_context* context, Ref<JSScript> script);
	};
}