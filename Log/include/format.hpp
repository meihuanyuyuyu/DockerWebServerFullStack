#ifndef __M_FMT_H__
#define __M_FMT_H__

#include "message.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <cassert>
#include <ctime>
namespace LogSys
{
    // 抽象格式化子项 基类
    class FormatItem
    {
    public:
        // 使用别名， using类型别名 = 原类型
        using ptr = std::shared_ptr<FormatItem>;
        virtual void format(std::ostream &out, const LogMsg &msg) = 0;
    };
    // 派生格式化子项 子类 - 日期 缩进 线程id 日志级别 日志器名称 文件名 行号 日志消息 换行
    // 数据
    class MsgFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << msg._pay_load;
        }
    };
    // 日志等级
    class LevelFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << LogLevel::toString(msg._level);
        }
    };
    // 时间
    class TimeFormatItem : public FormatItem
    {
    public:
        // 构造函数
        TimeFormatItem(std::string time_fmt = "%H:%M:%S")
            : _time_fmt(time_fmt)
        {
        }
        void format(std::ostream &out, const LogMsg &msg) override
        {
            // 时间结构体
            struct tm t;
            // 把时间戳放入定义的结构体中
            localtime_r(&msg._ctime, &t);
            char tmp[32] = {0};
            // 按照格式将时间戳转化为字符串
            strftime(tmp, 31, _time_fmt.c_str(), &t);
            out << tmp;
        }

    private:
        std::string _time_fmt; // 要显示的时间的格式，默认%H:%M:%S
    };
    // 文件名
    class FileFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << msg.file_name;
        }
    };
    // 行号
    class LineFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            // 数字转换为字符串
            out << msg._line;
        }
    };
    // 线程ID
    class ThreadFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << msg._t_id;
        }
    };
    // 日志器
    class LoggerFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << msg._logger;
        }
    };
    // 缩进
    class TabFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << "\t";
        }
    };
    // 换行
    class NLineFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << "\n";
        }
    };
    // []
    class OtherFormatItem : public FormatItem
    {
    public:
        OtherFormatItem(const std::string &str)
            : _str(str)
        {
        }
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << _str;
        }

    private:
        std::string _str;
    };


class Formatter
    {
    public:
        using ptr = std::shared_ptr<Formatter>;
        Formatter(const std::string &pattern= "[%d{%H:%M:%S}][%c][%f:%l][%p]%T%m%n");
        // 对msg进行格式化
        void format(std::ostream &out, const LogMsg &msg);
        std::string format(const LogMsg &msg);
        // 对格式化规则字符串解析，按顺序存放到_items中
        bool parsePattern();

    private:
        // 根据不同的格式化字符 创建对应的格式化子项对象
        FormatItem::ptr createItem(const std::string &key, const std::string &val);
    private:
        std::string _pattern;         // 格式化规则字符串
        std::vector<FormatItem::ptr> _items; // 每个需要格式化的类型
    };
}
#endif