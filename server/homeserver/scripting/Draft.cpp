#include "Draft.h"
#include "DraftSource.h"
// #include "lua/LuaDraft.h"

namespace scripting
{
	Draft::Draft(Ref<DraftSource> source)
		: state(DraftStates::kUncompiled), source(std::move(source))
	{ }
	Draft::~Draft()
	{ }
	Ref<Draft> Draft::Create(Ref<DraftSource> source)
	{
		assert(source != nullptr);

		switch (source->GetLanguage())
		{
			//case DraftLanguages::kLuaDraftLanguage:
				//return lua::LuaDraft::Create(source);
		default:
			LOG_ERROR("Invalid script language");
			return nullptr;
		}
	}

	ExecutionResult Draft::MakeReady()
	{
		// Skip checks if script is ready
		if (state == DraftStates::kInitialized)
			return ExecutionResult::Successful();

		// Compile script if needed
		if (state == DraftStates::kUncompiled)
		{
			ExecutionResult result = Compile();
			if (!result.IsSuccess())
			{
				LOG_ERROR("Compile script : {0}", result.GetError());
				return result;
			}
		}

		// Initialize script if needed
		if (state == DraftStates::kCompiled)
		{
			ExecutionResult result = Initialize();
			if (!result.IsSuccess())
			{
				LOG_ERROR("Initialize script : {0}", result.GetError());
				return result;
			}
		}

		// Check
		if (state != DraftStates::kInitialized)
			return ExecutionResult::Failed("Make ready");

		return ExecutionResult::Successful();
	}
}