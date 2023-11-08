#ifndef __M_LOGSYSTEM_H__
#define __M_LOGSYSTEM_H__

#include"log.hpp"
//全局接口


namespace LogSys{
    #define debug(fmt, ...) debug(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define info(fmt, ...) Logger::info(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define warn(fmt, ...) Logger::warn(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define error(fmt, ...) Logger::error(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define fatal(fmt, ...) Logger::fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

}


#endif