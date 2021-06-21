#pragma once
#include "common.h"

namespace scripting
{
	// Draft execution result
	// possibly containing error message if execution failed
	class ExecutionResult
	{
	private:
		std::string message;

		ExecutionResult()
			: message("")
		{}
		ExecutionResult(std::string message)
			: message(message)
		{}
	public:
		static ExecutionResult Successful() { return ExecutionResult(); }
		static ExecutionResult Failed(std::string message) { return ExecutionResult("Error: " + message); }

		bool IsSuccess() { return message.empty(); }
		std::string GetError() { return message; }
	};
}