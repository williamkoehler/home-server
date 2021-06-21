#pragma once
#include "common.h"
#include "signal/SignalManager.h"

namespace home
{
	class HOME_EXPORT Script
	{
	private:

	protected:

		Script();
		~Script();
	public:
		virtual std::string GetScriptName() = 0;
		virtual uint32_t GetScriptID() = 0;

		virtual bool OnInitialize() = 0;

		virtual void OnUpdate(Ref<SignalManager> signalManager, size_t cycle);

		virtual bool OnTerminate() = 0;
	};

	HOME_EXPORT uint32_t GenerateID(const char* name);

#define SCRIPT_CLASS(_class, _name) \
	public: \
		_class() : home::Script() {} \
		~_class() {} \
		static std::string _getScriptName() \
		{ \
			return _name; \
		} \
		virtual std::string GetScriptName() override \
		{ \
			return _name; \
		} \
		static uint32_t _getScriptID() \
		{ \
			static uint32_t scriptID = home::GenerateID(_name); \
			return scriptID; \
		} \
		virtual uint32_t GetScriptID() override \
		{ \
			return _getScriptID(); \
		} \
		static Ref<home::Script> _createScript() \
		{ \
			return boost::make_shared<_class>(); \
		}

	typedef Ref<home::Script>(CreateScriptFunction)();
}