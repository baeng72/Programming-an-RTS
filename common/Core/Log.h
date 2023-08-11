#pragma once
// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#pragma warning(disable:26451, disable:26819)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)
class Logger {
	static std::shared_ptr<spdlog::logger> spdlogger;
protected:
	Logger() {};
public:
	static void Init();
	/*static Logger* Instance();
	
	virtual void error(std::string fmt,...) = 0;
	virtual void warn(std::string fmt, ...) = 0;
	virtual void info(std::string fmt, ...) = 0;*/
	
	/*virtual void trace(std::string fmt, ...) = 0;
	virtual void critical(std::string fmt, ...) = 0;*/
	static std::shared_ptr<spdlog::logger> GetLogger() { return spdlogger; }
};

#define LOG_ERROR(...) Logger::GetLogger()->error(__VA_ARGS__)
#define LOG_WARN(...) Logger::GetLogger()->warn(__VA_ARGS__)
#define LOG_INFO(...) Logger::GetLogger()->info(__VA_ARGS__)
#define LOG_TRACE(...) Logger::GetLogger()->trace(__VA_ARGS__)
#define LOG_CRITICAL(...) Logger::GetLogger()->critical(__VA_ARGS__)
#ifdef NDEBUG
#define ASSERT(x,...)
#else
#define ASSERT(x,...) {if(!(x)) {LOG_ERROR("Assertion Failed: {0}",__VA_ARGS__); __debugbreak();}}
#endif