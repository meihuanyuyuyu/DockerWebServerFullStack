#include"log.hpp"
#include"loop.hpp"
#include"LogSystem.h"
#include"buffer.hpp"
#include"message.hpp"
#include"sink.hpp"
#include"format.hpp"


#include<sys/stat.h>


/*
扩展一个以时间作为日志文件滚动切换类型的日志落地模块：
    以时间进行文件滚动，实际上是以时间段进行滚动
    实现思想：以当前系统时间%时间段大小，得到当前时间的第几个时间段，判断当前文件的时间段是否一致
*/
// 落地方向：滚动文件（以时间滚动）
enum class TimeGap
{
    GAP_SEC,
    GAP_MIN,
    GAP_HOUR,
    GAP_DAY
};
class ROLLByTimeSink : public LogSys::LogSink
{
public:
    // 构造时传入文件名，并打开文件
    ROLLByTimeSink(const std::string &basename, TimeGap gaptype)
        : _basename(basename)
    {
        switch (gaptype)
        {
        case TimeGap::GAP_SEC:
            _gap_size = 1;
            break;
        case TimeGap::GAP_MIN:
            _gap_size = 60;
            break;
        case TimeGap::GAP_HOUR:
            _gap_size = 3600;
            break;
        case TimeGap::GAP_DAY:
            _gap_size = 3600 * 24;
            break;
        }
        // 判断创建的是第几个时间段
        // 特殊情况 任何数%1 都等于0，
        _cur_gap = _gap_size == 1 ? (size_t)time(nullptr) : (size_t)time(nullptr) % _gap_size;
        std::string filename = CreateFileName();
        // 1. 创建文件所在的目录
        LogSys::createDirectory(std::filesystem::path(filename).parent_path().string());
        // 2. 创建打开文件
        _ofs.open(filename, std::ios::binary | std::ios::app);
        assert(_ofs.is_open());
    }
    // 将日志消息写入到文件,
    void log(const char *data, size_t len)
    {
        // 判断使用的时候是第几个时间段
        time_t cur = time(nullptr);
        time_t cur_gap = _gap_size == 1 ?  time(nullptr) :  time(nullptr) % _gap_size;
        if (cur_gap != _cur_gap)
        {
            _ofs.close(); // 关闭文件
            std::string filename = CreateFileName();
            _ofs.open(filename, std::ios::binary | std::ios::app);
            assert(_ofs.is_open());
        }
        _ofs.write(data, len);
        assert(_ofs.good());
    }

private:
    std::string CreateFileName() // 进行大小判断，超过指定大小创建新文件名
    {
        // 获取系统时间重命名
        time_t t = time(nullptr);
        struct tm lt;
        localtime_r(&t, &lt);
        std::stringstream filename;
        filename << _basename;
        filename << lt.tm_year + 1900 << lt.tm_mon + 1 << lt.tm_mday << lt.tm_hour << lt.tm_min << lt.tm_sec;
        filename << ".log";
        return filename.str();
    }

private:
    // 通过基础文件名+扩展文件名（时间）生成一个实际的文件名
    std::string _basename;
    std::ofstream _ofs;
    size_t _cur_gap;  // 当前是第几个时间段
    size_t _gap_size; // 每个时间段的大小
};

void test_log()
{
    LogSys::Logger::ptr logger = LogSys::LoggerManager::getInstance().getLogger("async_logger");
    logger->debug(__FILE__, __LINE__, "%s", "测试日志");
    logger->info(__FILE__, __LINE__, "%s", "测试日志");
    logger->warn(__FILE__, __LINE__, "%s", "测试日志");
    logger->error(__FILE__, __LINE__, "%s", "测试日志");
    logger->fatal(__FILE__, __LINE__, "%s", "测试日志");

    size_t count = 0;
    while (count < 500000)
    {
        logger->fatal(__FILE__, __LINE__, "测试日志-%d", count++);
    }
}

int main()
{
    // std::cout << LogSys::Util::Date::GetTime() << std::endl;
    // std::string pathname = "./abc/bcd/a.txt";
    // LogSys::Util::File::CreateDirectory(LogSys::Util::File::Path(pathname));

    // std::cout << LogSys::LogLevel::toString(LogSys::LogLevel::value::DEBUG) << std::endl;
    // std::cout << LogSys::LogLevel::toString(LogSys::LogLevel::value::INFO) << std::endl;
    // std::cout << LogSys::LogLevel::toString(LogSys::LogLevel::value::WARN) << std::endl;
    // std::cout << LogSys::LogLevel::toString(LogSys::LogLevel::value::ERROR) << std::endl;
    // std::cout << LogSys::LogLevel::toString(LogSys::LogLevel::value::FATAL) << std::endl;
    // std::cout << LogSys::LogLevel::toString(LogSys::LogLevel::value::OFF) << std::endl;

    // LogSys::LogMsg msg(LogSys::LogLevel::value::DEBUG, __FILE__, __LINE__, "root", "格式化字符串功能测试...");
    // LogSys::Formatter fmt("abc%%abc[%d{%H:%M:%S}] %m%n");
    // LogSys::Formatter fmt;
    // std::string str = fmt.format(msg);
    // std::cout << str << std::endl;

    // LogSys::LogSink::ptr stdout_ptr = LogSys::SinkFactory::Create<LogSys::StdoutSink>();
    // LogSys::LogSink::ptr file_ptr = LogSys::SinkFactory::Create<LogSys::FileSink>("./logfile/test.log");
    // LogSys::LogSink::ptr roll_ptr = LogSys::SinkFactory::Create<LogSys::ROLLBySizeSink>("./logfile/roll-", 1024 * 1024);

    // stdout_ptr->log(str.c_str(), str.size());
    // file_ptr->log(str.c_str(), str.size());
    // size_t cur_size = 0;
    // size_t count = 0;
    // while (cur_size < 1024 * 1024 * 10)
    // {
    //     std::string tmp = "[" + std::to_string(count++) + "]" + str;
    //     roll_ptr->log(tmp.c_str(), tmp.size());
    //     cur_size += tmp.size();
    // }

    // LogSys::LogSink::ptr time_ptr = LogSys::SinkFactory::Create<ROLLByTimeSink>("./logfile/roll-", TimeGap::GAP_SEC);
    // size_t old = LogSys::Util::Date::GetTime();
    // while (LogSys::Util::Date::GetTime() < old + 5)
    // {
    //     time_ptr->log(str.c_str(), str.size());
    // }
    // std::string logger_name = "sync_logger";
    // LogSys::LogLevel::value limit = LogSys::LogLevel::value::WARN;
    // LogSys::Formatter::ptr fmt(new LogSys::Formatter("[%d{%H:%M:%S}][%c][%f:%l][%p]%T%m%n"));
    // LogSys::LogSink::ptr stdout_ptr = LogSys::SinkFactory::Create<LogSys::StdoutSink>();
    // LogSys::LogSink::ptr file_ptr = LogSys::SinkFactory::Create<LogSys::FileSink>("./logfile/test.log");
    // LogSys::LogSink::ptr roll_ptr = LogSys::SinkFactory::Create<LogSys::ROLLBySizeSink>("./logfile/roll-", 1024 * 1024);
    // std::vector<LogSys::LogSink::ptr> sinks{stdout_ptr, file_ptr, roll_ptr};
    // LogSys::Logger::ptr logger(new LogSys::SyncLogger(logger_name, limit, fmt, sinks));

    // std::unique_ptr<LogSys::LoggerBuilder> builder(new LogSys::LocalLoggerBuilder());
    // builder->buildLoggerName("async_logger");
    // builder->buildLoggerType(LogSys::LoggerType::LOGGER_ASYNC);
    // builder->buildLoggerLevel(LogSys::LogLevel::value::WARN);
    // builder->buildFormatter("[%c]%m%n");
    // // builder->buildEableUnSafeAsync();
    // builder->buildLoggerSink<LogSys::FileSink>("./logfile/async.log");
    // builder->buildLoggerSink<LogSys::StdoutSink>();
    // LogSys::Logger::ptr logger = builder->build();

    // logger->debug(__FILE__, __LINE__, "%s", "测试日志");
    // logger->info(__FILE__, __LINE__, "%s", "测试日志");
    // logger->warn(__FILE__, __LINE__, "%s", "测试日志");
    // logger->error(__FILE__, __LINE__, "%s", "测试日志");
    // logger->fatal(__FILE__, __LINE__, "%s", "测试日志");

    // size_t count = 0;
    // while (count < 500000)
    // {
    //     logger->fatal(__FILE__, __LINE__, "测试日志-%d", count++);
    // }


    // 读取文件数据，一点一点写入缓冲区，最终将缓冲区写入文件，判断新生成文件与源文件是否一致
    // std::ifstream ifs("./logfile/sync.log", std::ios::binary);
    // if(!ifs.is_open())
    // {
    //     std::cout << "open failed" << std::endl;
    //     return -1;
    // }
    // ifs.seekg(0, std::ios::end);// 将文件指针移到文件末尾
    // size_t fsize = ifs.tellg();// 当前位置相对于起始位置的偏移量，就是大小
    // ifs.seekg(0, std::ios::beg);// 重新跳转到起始位置
    // std::string body;
    // body.resize(fsize);
    // ifs.read(&body[0],fsize);
    // if(!ifs.good())
    // {
    //     std::cout << "read failed" << std::endl;
    //     return -1;
    // }
    // ifs.close();
    // std::cout << "fsize:" << fsize << std::endl;
    // LogSys::Buffer buffer;
    // for(int i = 0; i < body.size(); ++i)
    // {
    //     buffer.Push(&body[i],1);
    // }
    // std::cout << "reablesize:" << buffer.ReadAbleSize() << std::endl;
    // // buffer.Push(&body[0],body.size());
    // std::ofstream ofs("./logfile/tmp.log", std::ios::binary);
    // size_t rsize = buffer.ReadAbleSize();
    // for(int i = 0; i < rsize; ++i)
    // {
    //     ofs.write(buffer.Begin(), 1);
    //     buffer.MoveReader(1);
    // }
    // ofs.close();

    std::unique_ptr<LogSys::LoggerBuilder> builder(new LogSys::BuildAsyncLogger());
    builder->buildLoggerName("async_logger");
    builder->buildLoggerType(LogSys::LoggerType::LOGGER_ASYNC);
    builder->buildLoggerLevel(LogSys::LogLevel::level::WARN);
    builder->buildFormatter("[%c]%m%n");
    // builder->buildEableUnSafeAsync();
    builder->buildLoggerSink<LogSys::FileSink>("./logfile/async.log");
    builder->buildLoggerSink<LogSys::StdoutSink>();
    builder->build();

    test_log();
    return 0;
}