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

    void createDirectory(const std::string &pathname){
        bool flag = std::filesystem::create_directories(std::filesystem::path(pathname).parent_path());
    };

    class LogSink{
        public:
        using ptr=std::shared_ptr<LogSink>;
        LogSink(){};
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
        FileSink(const std::string & pathname):_pathname(pathname){
            createDirectory(_pathname);
            _ofs.open(_pathname,std::ios::binary|std::ios::app);
            assert(_ofs.is_open());
        }
        void log(const char *data,size_t len){
            _ofs.write(data,len);
            assert(_ofs.good());
        }
        private:
        std::string _pathname;
        std::ofstream _ofs;
    };
    class ROLLbySizeSink:public LogSink{
        ROLLbySizeSink(const std::string &basename, size_t max_size):_basename(basename),_max_size(max_size),_cur_size(0),_name_count(0){
            std::string pathanme = CreateFile();
            createDirectory(pathanme);
            _ofs.open(pathanme,std::ios::binary | std::ios::app);
            assert(_ofs.is_open());
        };
        void log(const char *data,size_t len){
            if(_cur_size>=_max_size){
                _ofs.close();
                std::string pathname = CreateFile();
                _ofs.open(pathname,std::ios::binary | std::ios::app);
                assert(_ofs.is_open());
                _cur_size = 0;
            }
            _ofs.write(data,len);
            assert(_ofs.good());
            _cur_size+=len;
        }
        private:
        std::string CreateFile(){
            time_t t = time(nullptr);
            struct tm *lt = localtime(&t);
            std::stringstream filename;
            filename<<_basename;
            filename<<lt->tm_year +1900<< lt->tm_mon + 1 << lt->tm_mday << lt->tm_hour << lt->tm_min << lt->tm_sec;
            filename<<"-"<<_name_count++;
            filename<<".log";
            return filename.str();
        }
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