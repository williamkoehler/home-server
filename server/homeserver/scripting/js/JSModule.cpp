#include "JSModule.hpp"

namespace scripting
{
	duk_ret_t JSModule::ScriptGetScriptID(duk_context* context)
	{
		assert(duk_get_top(context) == 0);

		duk_push_this(context);
		if (duk_get_prop_string(context, -1, DUK_HIDDEN_SYMBOL("script")))
		{
			Ref<JSScript> script = Ref<JSScript>((JSScript*)duk_get_pointer(context, -1));
			if (script == nullptr)
			{
				duk_pop_2(context); // Pop 'script' and this

				// Push id
				duk_push_int(context, script->GetSource()->GetSourceID());

				return 1;
			}
		}

		return duk_error(context, DUK_ERR_ERROR, "Invalid 'Script' object.");
	}

	void JSModule::JSLoadScriptModule(duk_context* context, Ref<JSScript> script)
	{
		assert(context != nullptr && script != nullptr);

		duk_push_object(context);

		static const duk_function_list_entry scriptFunctions[] = {
			{ "getScriptID", JSModule::ScriptGetScriptID, 0 },
			{ nullptr, nullptr, 0 }
		};

		duk_put_function_list(context, -1, scriptFunctions);

		//! A raw pointer is stored
		//! A javascript script must not be executed without a linked JSScript instance
		duk_push_pointer(context, script.get());
		duk_put_prop_string(context, -2, DUK_HIDDEN_SYMBOL("script"));

		duk_put_global_string(context, "Script");
	}
}