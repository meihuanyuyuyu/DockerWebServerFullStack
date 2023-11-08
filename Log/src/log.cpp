#include "log.hpp"
using namespace LogSys;

Logger::Logger(const std::string &logger_name, LogLevel::level lev, std::shared_ptr<Formatter> &format, std::vector<LogSink::ptr> &sinks) : _logger_name(logger_name), _limit_level(lev), _formatter(format), _sinks(sinks){};

void Logger::debug(const std::string &file, size_t line, const std::string &fmt, ...)
{
    if (LogLevel::level::DEBUG < _limit_level)
        return;
    va_list args;
    va_start(args, fmt);
    char *res;
    int ret = vasprintf(&res, fmt.c_str(), args);
    if (ret == -1)
        std::cout << "vasprint failed" << std::endl;
    va_end(args);
    serialize(LogLevel::level::DEBUG, file, line, res);
    free(res);
}

void Logger::info(const std::string &file, size_t line, const std::string &fmt, ...)
{
    if (LogLevel::level::INFO < _limit_level)
        return;
    va_list args;
    va_start(args, fmt);
    char *res;
    int ret = vasprintf(&res, fmt.c_str(), args);
    if (ret == -1)
        std::cout << "vasprint failed" << std::endl;
    va_end(args);
    serialize(LogLevel::level::INFO, file, line, res);
    free(res);
}

void Logger::warn(const std::string &file, size_t line, const std::string &fmt, ...)
{
    if (LogLevel::level::WARN < _limit_level)
        return;
    va_list args;
    va_start(args, fmt);
    char *res;
    int ret = vasprintf(&res, fmt.c_str(), args);
    if (ret == -1)
        std::cout << "vasprint failed" << std::endl;
    va_end(args);
    serialize(LogLevel::level::WARN, file, line, res);
    free(res);
}

void Logger::error(const std::string &file, size_t line, const std::string &fmt, ...)
{
    if (LogLevel::level::ERROR < _limit_level)
        return;
    va_list args;
    va_start(args, fmt);
    char *res;
    int ret = vasprintf(&res, fmt.c_str(), args);
    if (ret == -1)
        std::cout << "vasprint failed" << std::endl;
    va_end(args);
    serialize(LogLevel::level::ERROR, file, line, res);
    free(res);
}

void Logger::fatal(const std::string &file, size_t line, const std::string &fmt, ...)
{
    if (LogLevel::level::FATAL < _limit_level)
        return;
    va_list args;
    va_start(args, fmt);
    char *res;
    int ret = vasprintf(&res, fmt.c_str(), args);
    if (ret == -1)
        std::cout << "vasprint failed" << std::endl;
    va_end(args);
    serialize(LogLevel::level::FATAL, file, line, res);
    free(res);
}

inline std::string &Logger::name()
{
    return _logger_name;
}

void Logger::serialize(LogLevel::level l, const std::string &file, size_t line, char *str)
{
    std::lock_guard<std::mutex> lock(_lock);
    LogMsg msg(l, file, line, _logger_name, str);
    std::string res = _formatter->format(msg);
    for (auto &i : _sinks)
    {
        i->log(res.c_str(), res.size());
    }
}

SyncLogger::SyncLogger(const std::string &logger_name, LogLevel::level lev, Formatter::ptr &formatter, std::vector<LogSink::ptr> &sinks) : Logger(logger_name, lev, formatter, sinks){};

void SyncLogger::log(const char *data, size_t len)
{
    for (auto &i : _sinks)
    {
        i->log(data, len);
    }
}

AsyncLogger::AsyncLogger(const std::string &logger_name, LogLevel::level lev, std::shared_ptr<Formatter> &formatter, std::vector<std::shared_ptr<LogSink>> &sinks, AsyncType looper_type) : Logger(logger_name, lev, formatter, sinks)
{
    _looper = std::make_shared<AsyncLooper>(std::bind(&AsyncLogger::reallog, this, std::placeholders::_1), looper_type);
}

void AsyncLogger::log(const char *data, size_t len)
{
    _looper->push(data, len);
};

void AsyncLogger::reallog(Buffer &buf)
{
    if (_sinks.empty())
        return;
    for (auto &i : _sinks)
    {
        i->log(buf.Begin(), buf.ReadAbleSize());
    }
}

LoggerBuilder::LoggerBuilder() : _logger_name("default_logger"), _limit_level(LogLevel::level::DEBUG), _formatter(nullptr){};
void LoggerBuilder::buildLoggerLevel(LogLevel::level level) { _limit_level = level; };
void LoggerBuilder::buildLoggerName(const std::string &name) { _logger_name = name; };
void LoggerBuilder::buildFormatter(const std::string &pattern) { _formatter = std::make_shared<Formatter>(pattern); };
void LoggerBuilder::buildEableUnSafeAsync() { _looper_type = AsyncType::ASYNC_UNSAFE; };
void LoggerBuilder::buildLoggerType(LoggerType type){_logger_type = type;};

Logger::ptr BuildAsyncLogger::build()
{
    assert(!_logger_name.empty());
    if (_formatter.get() == nullptr)
        _formatter = std::make_shared<Formatter>();
    if (_sinks.empty())
        SinkFactory::create_sink<StdoutSink>();
    return std::make_shared<AsyncLogger>(_logger_name, _limit_level, _formatter, _sinks, _looper_type);
}

Logger::ptr BuildSyncLogger::build()
{
    assert(!_logger_name.empty());
    if (_formatter.get() == nullptr)
        _formatter = std::make_shared<Formatter>();
    if (_sinks.empty())
        buildLoggerSink<StdoutSink>();
    return std::make_shared<SyncLogger>(_logger_name, _limit_level, _formatter, _sinks);
}