#pragma once
#include "../common.hpp"

namespace scripting
{
	// Script execution result
	// Possibly containing error message if execution failed
	class ScriptResult
	{
	private:
		std::string message;

		ScriptResult()
			: message("")
		{}
		ScriptResult(std::string message)
			: message(message)
		{}
	public:
		static ScriptResult Success() { return ScriptResult(); }
		static ScriptResult Failure(std::string message) { return ScriptResult("Error: " + message); }

		bool IsSuccess() { return message.empty(); }
		std::string GetError() { return message; }
	};
}