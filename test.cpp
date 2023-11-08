#include"log.hpp"
#include"LogSystem.h"

int main(){

    std::unique_ptr<LogSys::LoggerBuilder> logger_builder(new LogSys::BuildSyncLogger());
    logger_builder->buildFormatter("[%d{%H:%M:%S}][%c][%f:%l][%p]%T%m%n");
    logger_builder->buildLoggerName("test_sync_loggger");
    logger_builder->buildLoggerSink<LogSys::FileSink>("./logfile/sync.log");
    logger_builder->buildLoggerSink<LogSys::StdoutSink>();
    logger_builder->buildLoggerLevel(LogSys::LogLevel::level::DEBUG);
    std::shared_ptr<LogSys::Logger> logger = logger_builder->build();
    logger->debug("do not call this func!");
    std::cout<< "finish build logger"<<std::endl;
}