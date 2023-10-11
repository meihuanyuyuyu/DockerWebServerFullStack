#ifndef __M_LOGSYSTEM_H__
#define __M_LOGSYSTEM_H__

#include"log.hpp"
//全局接口


namespace LogSys{
    Logger::ptr get_logger(const std::string &logger_name){return LoggerManager::getInstance().getLogger(logger_name);};
    Logger::ptr get_root_logger(){return LoggerManager::getInstance().rootLogger();};
    #define debug(fmt, ...) debug(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define info(fmt, ...) info(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define warn(fmt, ...) warn(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define error(fmt, ...) error(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define fatal(fmt, ...) fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__)


    #define DEBUG(fmt, ...) Log_System::rootLogger()->debug(fmt, ##__VA_ARGS__)
    #define INFO(fmt, ...) Log_System::rootLogger()->info(fmt, ##__VA_ARGS__)
    #define WARN(fmt, ...) Log_System::rootLogger()->warn(fmt, ##__VA_ARGS__)
    #define ERROR(fmt, ...) Log_System::rootLogger()->error(fmt, ##__VA_ARGS__)
    #define FATAL(fmt, ...) Log_System::rootLogger()->fatal(fmt, ##__VA_ARGS__)
}


#endif