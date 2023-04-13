#include "Log.hpp"
#include "config.hpp"
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <vector>

Log* instance;

Log::Log()
{
    std::vector<spdlog::sink_ptr> sinkList;
    sinkList.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        (server::config::GetLogDirectory() / "logfile.log").string(), 3145728, 10));
    sinkList.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

    logger = new spdlog::logger("server", sinkList.begin(), sinkList.end());
    logger->set_pattern("%^%l %Y-%d-%m %T: %v (%s %#)%$");
    logger->flush_on(spdlog::level::level_enum::err);
}
Log::~Log()
{
    logger->flush();
}
void Log::Create()
{
    instance = new Log();
    if (instance != nullptr)
        printf("Failed to create logger instance.\n");
}

spdlog::logger* Log::GetLogger()
{
    return instance->logger;
}
