#include"loop.hpp"
#include"log.hpp"
#include"LogSystem.h"
#include<sys/stat.h>
#include<time.h>
#include<atomic>
#include<mutex>
#include<condition_variable>

#define THREAD_NUM 5000


int count(0);
std::condition_variable cond;
std::mutex mtx;
int is_finish(0);
std::mutex mtx2;
std::condition_variable cond2;

void func(LogSys::Logger::ptr &logger){
   {
        std::unique_lock<std::mutex> lock(mtx);
        cond.wait(lock,[&]{{return count==THREAD_NUM;}});
   }
    logger->info("thread%lu:",std::this_thread::get_id());
    logger->debug("hello world!");
    logger->error("hello world!");
    {
        std::unique_lock<std::mutex> lock(mtx2);
        is_finish++;
    }
    if(is_finish==THREAD_NUM)
        cond2.notify_all();
    return;
}

void SyncLogtest(int thread_num){
    LogSys::BuildSyncLogger builder = LogSys::BuildSyncLogger();
    builder.buildFormatter("[%d{%H:%M:%S}][%c][%f:%l][%p]%T%m%n");
    builder.buildLoggerSink<LogSys::FileSink>("./logsys.log");
    builder.buildLoggerName("SyncLogtest");
    builder.buildLoggerLevel(LogSys::LogLevel::level::DEBUG);
    LogSys::Logger::ptr sync_logger = builder.build();
    std::cout<<"finish build logger!"<<std::endl;
    std::cout<<"start to test logger:"<<std::endl;


    clock_t start_time = clock();
    std::vector<std::thread> threads(thread_num);
    for(auto i=0;i<thread_num;i++){
        threads[i] = std::thread(func,std::ref(sync_logger));
        count++;
        threads[i].detach();
    }
     cond.notify_all();
     {
        std::unique_lock<std::mutex> lock(mtx2);
        cond2.wait(lock,[&]{{return is_finish==THREAD_NUM;}});
     }
    clock_t end_time = clock();
    std::cout<< thread_num << "个线程并发,"<<"time cost:"<< float(end_time-start_time)/float(CLOCKS_PER_SEC)<<"ms"<<std::endl;
    //std::cout<< "QPS:"<<thread_num/long((end_time-start_time)/CLOCKS_PER_SEC)<<std::endl;
}

void AsyncLog(int thread_num){
    LogSys::BuildAsyncLogger builder = LogSys::BuildAsyncLogger();
    builder.buildFormatter("[%d{%H:%M:%S}][%c][%f:%l][%p]%T%m%n");
    builder.buildLoggerSink<LogSys::FileSink>("./logsys.log");
    builder.buildLoggerName("AsyncLogtest");
    builder.buildLoggerLevel(LogSys::LogLevel::level::DEBUG);
    LogSys::Logger::ptr async_logger = builder.build();
    std::cout<<"finish build logger!"<<std::endl;
    std::cout<<"start to test logger:"<<std::endl;
    clock_t start_time = clock();
    std::vector<std::thread> threads(thread_num);
    for(auto i=0;i<thread_num;i++){
        threads[i] = std::thread(func,std::ref(async_logger));
        count++;
        threads[i].detach();
    }
    cond.notify_all();
    {
        std::unique_lock<std::mutex> lock(mtx2);
        cond2.wait(lock,[&]{{return is_finish==THREAD_NUM;}});
    }
    clock_t end_time = clock();
    std::cout<< thread_num << "个线程并发,"<<"time cost:"<<float(end_time-start_time)/float(CLOCKS_PER_SEC)<<std::endl;
    //std::cout<< "QPS:"<<thread_num/long((end_time-start_time)/CLOCKS_PER_SEC)<<std::endl;

}

int main()
{
    SyncLogtest(THREAD_NUM);
    count = 0;
    is_finish = 0;
    AsyncLog(THREAD_NUM);
}