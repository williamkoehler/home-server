#pragma once
#include "common.h"
#include "Result.h"
#include "DraftManager.h"
#include "DraftSource.h"
#include <visual/Visual.h>
#include <helper/Types.h>

namespace server
{
	class JsonApi;
}

namespace scripting
{
	class Field;

	// Share with globals
	/*namespace lua
	{
		class LuaGlobals;
	}*/

	enum class DraftStates
	{
		// Source code is available but not compiled
		// Source code needs to be compiled
		kUncompiled,

		// Source code is available and is compiled
		// Draft needs to be initialized
		kCompiled,

		// Source code is available but cannot be compiled or run
		// Source code needs to be fixed
		kBroken,

		// Draft was initialized
		// Draft is ready to use
		kInitialized,

		// Draft was terminated
		// To execute it needs to be reinitialized
		kTerminated,
	};

	class Draft : public boost::enable_shared_from_this<Draft>
	{
	protected:
		//friend class lua::LuaGlobals;
		friend class server::JsonApi;

		boost::mutex mutex;

		boost::atomic<DraftStates> state;

		const Ref<DraftSource> source;

		// Properties
		boost::unordered_map<identifier_t, Ref<Field>> fieldList;

		Draft(Ref<DraftSource> source);
		~Draft();
	public:
		static Ref<Draft> Create(Ref<DraftSource> source);

		virtual DraftLanguages GetLanguage() = 0;

		inline DraftStates GetState() { return state; }

		inline uint32_t GetSourceID()
		{
			boost::lock_guard lock(mutex);
			return source->GetSourceID();
		}

		// Reset state
		inline void Reset() { state = DraftStates::kUncompiled; }

		// Compile action script source code
		virtual ExecutionResult Compile() = 0;

		// Make script ready for execution
		ExecutionResult MakeReady();

		// Call script initialize method
		// Must be called before execution
		virtual ExecutionResult Initialize() = 0;

		// Call script execute method
		// Can only be called when script is loaded/updated
		virtual ExecutionResult Execute() = 0;

		// Call script terminate method
		virtual ExecutionResult Terminate() = 0;
	};
}