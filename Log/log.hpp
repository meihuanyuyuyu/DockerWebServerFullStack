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

namespace LogSys
{
    class Logger
    {
    public:
        using ptr = std::shared_ptr<Logger>;
        Logger(const std::string &logger_name, LogLevel::level lev, std::shared_ptr<Formatter> &format, std::vector<LogSink::ptr> &sinks) : _logger_name(logger_name), _limit_level(lev), _formatter(format), _sinks(sinks){};
        void debug(const std::string &file, size_t line, const std::string &fmt, ...)
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
        void info(const std::string &file, size_t line, const std::string &fmt, ...)
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
        };
        void warn(const std::string &file, size_t line, const std::string &fmt, ...)
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
        };
        void error(const std::string &file, size_t line, const std::string &fmt, ...)
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
        };
        void fatal(const std::string &file, size_t line, const std::string &fmt, ...)
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
        };
        constexpr std::string &name() { return _logger_name; };

    protected:
        void serialize(LogLevel::level l, const std::string &file, size_t line, char *str)
        {
            LogMsg msg(l, file, line, _logger_name, str);
            std::stringstream ss;
            _formatter->format(ss, msg);
            log(ss.str().c_str(), ss.str().size());
        }
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
        SyncLogger(const std::string &logger_name, LogLevel::level lev, std::shared_ptr<Formatter> &formatter, std::vector<std::shared_ptr<LogSink>> &sinks) : Logger(logger_name, lev, formatter, sinks){};

    protected:
        void log(const char *data, size_t len)
        {
            std::unique_lock<std::mutex> lock(_lock);
            if (_sinks.empty())
                return;
            for (auto &sink : _sinks)
            {
                sink->log(data, len);
            }
        }
    };
    class AsyncLogger : public Logger
    {
    public:
        AsyncLogger(const std::string &logger_name, LogLevel::level lev, std::shared_ptr<Formatter> &formatter, std::vector<std::shared_ptr<LogSink>> &sinks) : Logger(logger_name, lev, formatter, sinks){};

    protected:
        void log(const char *data, size_t len)
        {
            _looper->push(data, len);
        };
        void reallog(Buffer &buf)
        {
            if (_sinks.empty())
                return;
            for (auto &sink : _sinks)
            {
                sink->log(buf.Begin(), buf.ReadAbleSize());
            }
        }

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
        LoggerBuilder() : _logger_type(LoggerType::LOGGER_SYNC), _limit_level(LogLevel::level::DEBUG){};
        void buildLoggerType(LoggerType type)
        {
            _logger_type = type;
        }
        void buildEableUnSafeAsync()
        {
            _looper_type = AsyncType::ASYNC_UNSAFE;
        }
        void buildLoggerName(const std::string &name)
        {
            _logger_name = name;
        }
        void buildLoggerLevel(LogLevel::level level)
        {
            _limit_level = level;
        }
        void buildFormatter(const std::string &pattern)
        {
            _formatter = std::make_shared<Formatter>(pattern);
        }
        template <class SinkType, class... Args>
        void buildLoggerSink(Args &&...args)
        {
            LogSink::ptr psink = SinkFactory::create_sink<SinkType>(std::forward<Args>(args)...);
            _sinks.push_back(psink);
        }

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
        virtual Logger::ptr build() override
        {
            assert(!_logger_name.empty());
            if (_formatter.get() == nullptr)
                _formatter = std::make_shared<Formatter>();
            if (_sinks.empty())
                SinkFactory::create_sink<StdoutSink>();
            return std::make_shared<SyncLogger>(_logger_name, _limit_level, _formatter, _sinks);
        };
    };


    class BuildAsyncLogger : public LoggerBuilder
    {
        virtual Logger::ptr build() override
        {
            assert(!_logger_name.empty());
            if (_formatter.get() == nullptr)
                _formatter = std::make_shared<Formatter>();
            if (_sinks.empty())
                SinkFactory::create_sink<StdoutSink>();
            return std::make_shared<AsyncLogger>(_logger_name, _limit_level, _formatter, _sinks);
        };
    };

    
    class LoggerManager
    {
    public:
        static LoggerManager &getInstance()
        {
            static LoggerManager logger_mger;
            return logger_mger;
        }
        bool hasLogger(const std::string &name){
            std::unique_lock<std::mutex> lock(_lock);
            return _loggers.find(name)==_loggers.end()?false:true;
        }
        void addLogger(Logger::ptr &logger){
            if(hasLogger(logger->name())) return;
            std::unique_lock<std::mutex> lock(_lock);
            _loggers.insert(std::make_pair(logger->name(),logger));
        }

        Logger::ptr getLogger(const std::string &name){
            std::unique_lock<std::mutex> lock(_lock);
            auto it = _loggers.find(name);
            if(it==_loggers.end()) return nullptr;
            return it->second;
        }
        Logger::ptr rootLogger(){
            return _default_logger;
        }

    private:
        LoggerManager()
        {
            std::unique_ptr<LoggerBuilder> builder(new BuildSyncLogger());
            builder->buildLoggerName("root");
            _default_logger = builder->build();
            _loggers.insert(std::make_pair("root", _default_logger));
        };

        std::mutex _lock;
        Logger::ptr _default_logger;
        std::unordered_map<std::string, Logger::ptr> _loggers;
    };

} // namespace LogSys

#endif
