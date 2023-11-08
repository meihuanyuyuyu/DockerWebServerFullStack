#ifndef LOG_SYS_H
#define LOG_SYS_H

#include <sstream>
#include "format.hpp"
#include "message.hpp"
#include "sink.hpp"
#include "buffer.hpp"
#include "loop.hpp"
#include <sys/stat.h>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <stdarg.h>

// logger类设计:
// 构造输入const string logname, level lev, shared_ptr<Formatter> format, vector<shared_ptr<LogSink>> sinks

namespace LogSys
{
    class Logger
    {
    public:
        using ptr = std::shared_ptr<Logger>;
        Logger(const std::string &logger_name, LogLevel::level lev, std::shared_ptr<Formatter> &format, std::vector<LogSink::ptr> &sinks);
        void debug(const std::string &file, size_t line, const std::string &fmt, ...);
        void info(const std::string &file, size_t line, const std::string &fmt, ...);
        void warn(const std::string &file, size_t line, const std::string &fmt, ...);
        void error(const std::string &file, size_t line, const std::string &fmt, ...);
        void fatal(const std::string &file, size_t line, const std::string &fmt, ...);
        std::string &name();

    protected:
        void serialize(LogLevel::level l, const std::string &file, size_t line, char *str);
        virtual void log(const char *data, size_t len) = 0;

    protected:
        std::mutex _lock;
        std::string _logger_name;
        std::shared_ptr<Formatter> _formatter;
        std::vector<LogSink::ptr> _sinks;
        std::atomic<LogLevel::level> _limit_level;
    };

    // 上锁同步落地日志
    class SyncLogger : public Logger
    {
    public:
        SyncLogger(const std::string &logger_name, LogLevel::level lev, Formatter::ptr &formatter, std::vector<LogSink::ptr> &sinks);

    protected:
        void log(const char *data, size_t len);
    };

    class AsyncLogger : public Logger
    {
    public:
        AsyncLogger(const std::string &logger_name, LogLevel::level lev, std::shared_ptr<Formatter> &formatter, std::vector<std::shared_ptr<LogSink>> &sinks, AsyncType looper_type);

    protected:
        void log(const char *data, size_t len);
        void reallog(Buffer &buf);

    private:
        AsyncLooper::ptr _looper;
    };

    // 建造者模式构建日志器
    enum class LoggerType
    {
        LOGGER_SYNC,
        LOGGER_ASYNC
    };

    class LoggerBuilder
    {
    public:
        LoggerBuilder();
        void buildLoggerType(LoggerType type);
        void buildEableUnSafeAsync();
        void buildLoggerName(const std::string &name);
        void buildLoggerLevel(LogLevel::level level);
        void buildFormatter(const std::string &pattern);
        template <class SinkType, class... Args>
        void buildLoggerSink(Args &&...args)
        {
            LogSink::ptr sink = SinkFactory::create_sink<SinkType>(std::forward<Args>(args)...);
            _sinks.push_back(sink);
        };
        virtual Logger::ptr build() = 0;

    protected:
        LoggerType _logger_type;
        AsyncType _looper_type;
        std::string _logger_name;
        LogLevel::level _limit_level;
        Formatter::ptr _formatter;
        std::vector<LogSink::ptr> _sinks;
    };

    class BuildSyncLogger : public LoggerBuilder
    {
    public:
        virtual Logger::ptr build() override;
    };

    class BuildAsyncLogger : public LoggerBuilder
    {
    public:
        virtual Logger::ptr build() override;
    };
}
#endif
