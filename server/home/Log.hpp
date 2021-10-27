#pragma once
#define SPDLOG_HEADER_ONLY
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

#define LOG_INFO(format, ...) Log::GetLogger()->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::info, format, ##__VA_ARGS__)
#ifdef _DEBUG
#define LOG_DEBUG_INFO(format, ...) Log::GetLogger()->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::info, format, ##__VA_ARGS__)
#else
#define LOG_DEBUG_INFO
#endif

#define LOG_WARNING(format, ...) Log::GetLogger()->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::warn, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) Log::GetLogger()->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::err, format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...)	{ Log::GetLogger()->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::critical, format, ##__VA_ARGS__); \
								throw std::runtime_error("Fatal error"); }
#define LOG_CODE_MISSING(format, ...) Log::GetLogger()->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::warn, "CODE MISSING" # format, ##__VA_ARGS__)
#define LOG_FLUSH() Log::GetLogger()->flush()