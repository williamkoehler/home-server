#pragma once
#define SPDLOG_LEVEL_NAMES	{ \
								"TRACE", \
								"DEBUG", \
								"INFO ", \
								"WARN ", \
								"ERROR", \
								"ERROR", \
								"OFF  " \
							}
#include <spdlog/spdlog.h>

class Log
{
private:
	spdlog::logger* logger;

public:
	Log();
	~Log();

	static spdlog::logger* GetLogger();
};

#define LOG_INFO(format, ...) Log::GetLogger()->info(format, ##__VA_ARGS__)
#ifdef _DEBUG
#define LOG_DEBUG_INFO(format, ...) Log::GetLogger()->info(format, ##__VA_ARGS__)
#else
#define LOG_DEBUG_INFO
#endif

#define LOG_WARNING(format, ...) Log::GetLogger()->warn(format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) Log::GetLogger()->error(format, ##__VA_ARGS__)
#define LOG_EXCEPTION(format, ...) Log::GetLogger()->error("Exception : " format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...)	{ Log::GetLogger()->critical(format, ##__VA_ARGS__); \
								throw std::runtime_error("Fatal error"); }
#define LOG_CODE_MISSING(file) Log::GetLogger()->warn(file ": CODE MISSING")
#define LOG_MEMORY_ALLOCATION(format, ...) Log::GetLogger()->error("Memory allocation : " format, ##__VA_ARGS__)
#define LOG_FLUSH() Log::GetLogger()->flush()