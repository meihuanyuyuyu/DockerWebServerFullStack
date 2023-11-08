#include"sink.hpp"
using namespace LogSys;

void LogSys::createDirectory(const std::string &pathname){
    bool flag = std::filesystem::create_directories(std::filesystem::path(pathname).parent_path());
}

LogSink::LogSink(){};


FileSink::FileSink(const std::string & pathname):_pathname(pathname){
    createDirectory(_pathname);
    _ofs.open(_pathname,std::ios::binary|std::ios::app);
    assert(_ofs.is_open());
}

void FileSink::log(const char *data,size_t len){
    _ofs.write(data,len);
    assert(_ofs.good());
}

ROLLbySizeSink::ROLLbySizeSink(const std::string &basename,size_t max_size):_basename(basename),_max_size(max_size),_cur_size(0),_name_count(0){
    std::string pathname = CreateFile();
    createDirectory(pathname);
    _ofs.open(pathname,std::ios::binary | std::ios::app);
    assert(_ofs.is_open());
};

void ROLLbySizeSink::log(const char *data,size_t len){
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

std::string ROLLbySizeSink::CreateFile(){
    time_t t = time(nullptr);
    struct tm *lt = localtime(&t);
    std::stringstream filename;
    filename<<_basename;
    filename<<lt->tm_year +1900<< lt->tm_mon + 1 << lt->tm_mday << lt->tm_hour << lt->tm_min << lt->tm_sec;
    filename<<"-"<<_name_count++;
    filename<<".log";
    return filename.str();
}

