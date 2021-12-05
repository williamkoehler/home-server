#pragma once
#include "../../common.hpp"
#include "duktape.h"

namespace server
{
	namespace javascript
	{
		class JSActionModule
		{
		public:
			/// @brief Import javacscript action module.
			/// The action module grants access to an action instance through a global variable.
			/// @param context Duktape context
			/// @param actionID Action unique id
			/// @return Successfulness
			static bool Import(duk_context* context, identifier_t actionID);
		};
	}
}