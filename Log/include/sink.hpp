#ifndef __M_SINK_H__
#define __M_SINK_H__
#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<memory>
#include<cassert>
#include<filesystem>
#include<sys/stat.h>
#include<errno.h>


namespace LogSys{

    void createDirectory(const std::string &pathname);
    
    class LogSink{
        public:
        using ptr=std::shared_ptr<LogSink>;
        LogSink();
        virtual ~LogSink() {};
        virtual void log(const char *data, size_t len)=0;
    };

    class StdoutSink:public LogSink{
        void log(const char *data, size_t len){
            std::cout.write(data,len);
        }
    };

    class FileSink:public LogSink{
        public:
        FileSink(const std::string & pathname);
        void log(const char *data,size_t len);
        private:
        std::string _pathname;
        std::ofstream _ofs;
    };

    class ROLLbySizeSink:public LogSink{
        ROLLbySizeSink(const std::string &basename, size_t max_size);
        void log(const char *data,size_t len);
        private:
        std::string CreateFile();
        private:
        std::string _basename;
        size_t _name_count;
        std::ofstream _ofs;
        size_t _max_size;
        size_t _cur_size;
    };

class SinkFactory{
    public:
        template<typename SinkT, class ...Args>
        static LogSink::ptr create_sink(Args &&...args){
            return std::make_shared<SinkT>(std::forward<Args>(args)...); 
        }
};
}

#endif