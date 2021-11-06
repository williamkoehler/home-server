#include "Script.hpp"
#include "ScriptManager.hpp"

namespace server
{
	Script::Script(Ref<ScriptSource> source)
		: source(source)
	{
	}
	Script::~Script()
	{
	}
}