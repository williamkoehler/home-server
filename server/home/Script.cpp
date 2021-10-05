#include "Script.hpp"

namespace home
{
	Script::Script()
	{
	}
	Script::~Script()
	{
	}

	void Script::OnUpdate(Ref<SignalManager> signalManager, size_t cycle) { }

	uint32_t GenerateID(const char* name)
	{
		return XXH32(name, strlen(name), 0x53435249);
	}
}