
#include "Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
std::shared_ptr<spdlog::logger> Logger::spdlogger;

void Logger::Init() {
	std::vector<spdlog::sink_ptr> logSinks;
	logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Debug.log", true));
	logSinks[0]->set_pattern("%^[%T] %n: %v%$");
	logSinks[1]->set_pattern("[%T] [%l] %n: %v");
	spdlogger = std::make_shared<spdlog::logger>("Debug", begin(logSinks), end(logSinks));
	spdlog::register_logger(spdlogger);
	spdlogger->set_level(spdlog::level::trace);
	spdlogger->flush_on(spdlog::level::trace);
}

//class SpdLogger : public Logger {
//	std::shared_ptr<spdlog::logger> spdlogger;
//public:
//	SpdLogger();
//	virtual void error(std::string fmt, ...)override;
//	virtual void warn(std::string fmt, ...) override;
//	virtual void info(std::string fmt, ...) override;
//	virtual void trace(std::string fmt, ...) override;
//	virtual void critical(std::string fmt, ...) override;
//};
//SpdLogger::SpdLogger() {
//	std::vector<spdlog::sink_ptr> logSinks;
//	logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
//	logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Debug.log", true));
//	logSinks[0]->set_pattern("%^[%T] %n: %v%$");
//	logSinks[1]->set_pattern("[%T] [%l] %n: %v");
//	spdlogger = std::make_shared<spdlog::logger>("Debug", begin(logSinks), end(logSinks));
//	spdlog::register_logger(spdlogger);
//	spdlogger->set_level(spdlog::level::trace);
//	spdlogger->flush_on(spdlog::level::trace);
//}
//void SpdLogger::error(std::string fmt, ...) {
//
//	std::va_list args;
//	va_start(args, fmt);
//	spdlogger->error(fmt,args);
//	va_end(args);
//}
//void SpdLogger::warn(std::string fmt, ...) {
//	std::va_list args;
//	va_start(args, fmt);
//	spdlogger->warn(fmt,args);
//	va_end(args);
//}
//void SpdLogger::info(std::string fmt, ...) {
//	std::va_list args;
//	va_start(args, fmt);
//	spdlogger->info(fmt,args);
//	va_end(args);
//}
//void SpdLogger::trace(std::string fmt, ...) {
//	std::va_list args;
//	va_start(args, fmt);
//	spdlogger->trace(fmt,args);
//	va_end(args);
//}
//void SpdLogger::critical(std::string fmt, ...) {
//	std::va_list args;
//	va_start(args, fmt);
//	spdlogger->critical(fmt,args);
//	va_end(args);
//}
//SpdLogger* logger{ nullptr };
//Logger* Logger::Instance() {
//	if (logger == nullptr)
//		logger = new SpdLogger();
//	return logger;
//}
//
//
//
