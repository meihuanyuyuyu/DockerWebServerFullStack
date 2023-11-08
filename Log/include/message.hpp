#ifndef __M_MSG_H__
#define __M_MSG_H__

#include <string>
#include <time.h>
#include <thread>

namespace LogSys
{
    class LogLevel
    {
    private:
        /* data */
    public:
        enum class level
        {
            UNKNOW = 0,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
            OFF
        };
        static const char *toString(LogLevel::level l)
        {
            switch (l)
            {
            case LogLevel::level::DEBUG:
                return "DEBUG";
            case LogLevel::level::INFO:
                return "INFO";
            case LogLevel::level::WARN:
                return "WARN";
            case LogLevel::level::ERROR:
                return "ERROR";
            case LogLevel::level::FATAL:
                return "FATAL";
            case LogLevel::level::OFF:
                return "OFF";
            }
            return "UNKNOW";
        }
    };

    struct LogMsg
    {
        LogMsg(LogLevel::level l, std::string file, size_t line, std::string logger, std::string payload) : _ctime((size_t)time(nullptr)), _level(l), file_name(file), _line(line), _pay_load(payload), _t_id(std::this_thread::get_id()), _logger(logger){};
        time_t _ctime;
        std::string file_name;
        size_t _line;
        std::string _pay_load;
        std::string _logger;
        LogLevel::level _level;
        std::thread::id _t_id;
    };
}

#endif