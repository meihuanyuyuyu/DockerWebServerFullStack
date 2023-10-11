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

    /*
        %d 日期 包含子格式{%H:%M:%S}
        %t 线程id
        %c 日志器名称
        %f 文件名
        %l 行号
        %p 日志级别
        %T 缩进
        %m 日志消息
        %n 换行
    */
    class Formatter
    {
    public:
        using ptr = std::shared_ptr<Formatter>;
        Formatter(const std::string &pattern = "[%d{%H:%M:%S}][%t][%c][%f:%l][%p]%T%m%n")
            : _pattern(pattern)
        {
            // 传入的pattern首先要解析，解析成功才可以输出
            assert(parsePattern());
        }
        // 对msg进行格式化
        void format(std::ostream &out, const LogMsg &msg)
        {
            std::cout<<"Format:";
            for (auto &item : _items)
            {
                // 把std::vector<FormatItem::ptr> _items;中按顺序存放要格式化的消息
                // msg是具体信息，按照格式流入out
                item->format(out, msg); // 调用的是FormatItem的format，根据不同的格式调用不同子类的format
            }
        }

        std::string format(const LogMsg &msg)
        {
            std::stringstream ss;
            // 把msg放入ss输入流中
            format(ss, msg); // 调用的是Formatter中的 void format(std::ostream &out, const LogMsg &msg)
            return ss.str();
        }
        // 对格式化规则字符串解析，按顺序存放到_items中
        bool parsePattern()
        {
            // 1. 对格式化规则字符串进行解析
            // abcde[%d{%H:%M:%S}][%p]%T%m%n
            std::vector<std::pair<std::string, std::string>> fmt_order;
            size_t pos = 0;
            std::string key, val;
            while (pos < _pattern.size())
            {
                // 首先判断是否为%，不是的话就是其他字符，直接存入
                if (_pattern[pos] != '%')
                {
                    val.push_back(_pattern[pos++]);
                    continue;
                }
                // 走到这里表示找到了%字符
                // 可能后面是%，直接打印，也可能后面是格式化字符，
                if (pos + 1 < _pattern.size() && _pattern[pos + 1] == '%')
                {
                    val.push_back('%');
                    pos += 2;
                    continue;
                }
                // 走到这里说明是格式化字符,可能是新的格式化字符，需要将前面的原始字符串都添加到数组中
                if (!val.empty())
                {
                    fmt_order.push_back(std::make_pair("", val));
                    val.clear();
                }
                // 处理好%之前的字符，开始处理%之后的格式化字符
                pos += 1;                   // 这时pos指向‘格式化字符的位置
                if (pos == _pattern.size()) // 判断是否合法
                {
                    // %之后没有数据，说明输入字符串有问题，出错返回
                    std::cout << "%之后没有合适的格式化字符！" << std::endl;
                    return false;
                }
                // 插入格式化字符，进行创建
                key = _pattern[pos++];
                // %t格式化字符串后如果是{}，说明还有子串，继续插入
                if (pos < _pattern.size() && _pattern[pos] == '{')
                {
                    // 开始插入子串中的字符
                    pos += 1;                                             // 这时pos指向'}'之后的位置
                    while (pos < _pattern.size() && _pattern[pos] != '}') // 表示在整个字符串范围内，其在子串范围内
                    {
                        val.push_back(_pattern[pos++]);
                    }
                    // 如果是超过整个字符串范围跳出了while循环，说明没有找到'}'
                    if (pos == _pattern.size())
                    {
                        std::cout << "子规则{}匹配错误！" << std::endl;
                        return false;
                    }
                    // 不是上面的情况则是正常跳出循环
                    pos += 1;
                }
                fmt_order.push_back(std::make_pair(key, val));
                key.clear();
                val.clear();
            }
            // 2. 根据解析得到的数据初始化子项数组成员
            for (auto &it : fmt_order)
            {
                _items.push_back(createItem(it.first, it.second));
            }
            return true;
        }

    private:
        // 根据不同的格式化字符 创建对应的格式化子项对象
        FormatItem::ptr createItem(const std::string &key, const std::string &val)
        {
            if (key == "d")
                return std::make_shared<TimeFormatItem>(val);
            if (key == "t")
                return std::make_shared<ThreadFormatItem>();
            if (key == "c")
                return std::make_shared<LoggerFormatItem>();
            if (key == "f")
                return std::make_shared<FileFormatItem>();
            if (key == "l")
                return std::make_shared<LineFormatItem>();
            if (key == "p")
                return std::make_shared<LevelFormatItem>();
            if (key == "T")
                return std::make_shared<TabFormatItem>();
            if (key == "m")
                return std::make_shared<MsgFormatItem>();
            if (key == "n")
                return std::make_shared<NLineFormatItem>();
            if (key.empty())
                return std::make_shared<OtherFormatItem>(val);
            std::cout << "没有对应的格式化字符，%" << key << std::endl;
            abort();
            return FormatItem::ptr();
        }

    private:
        std::string _pattern;                // 格式化规则字符串
        std::vector<FormatItem::ptr> _items; // 每个需要格式化的类型
    };
}
#endif