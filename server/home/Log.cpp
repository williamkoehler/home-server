#include <vector>
#include "Log.h"
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/msvc_sink.h>

HOME_EXPORT Log instance = Log();

Log::Log()
{
	std::vector<spdlog::sink_ptr> sinkList;
	sinkList.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logfile.log", 3145728, 5));
	sinkList.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
#ifdef _WIN32
	sinkList.push_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
#endif

	logger = new spdlog::logger("server", sinkList.begin(), sinkList.end());
	logger->set_pattern("%^%l %Y-%d-%m %T : %v%$");
	logger->flush_on(spdlog::level::level_enum::err);
}
Log::~Log()
{
	logger->flush();
}

spdlog::logger* Log::GetLogger()
{
	return instance.logger;
}
