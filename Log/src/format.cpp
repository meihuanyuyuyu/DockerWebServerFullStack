#include "format.hpp"

LogSys::Formatter::Formatter(const std::string &pattern)
    : _pattern(pattern)
{
    // 传入的pattern首先要解析，解析成功才可以输出
    assert(parsePattern());
}

void LogSys::Formatter::format(std::ostream &out, const LogMsg &msg)
{
    for (auto &item : _items)
    {
        item->format(out, msg);
    }
}

std::string LogSys::Formatter::format(const LogMsg &msg)
{
    std::stringstream ss;
    for (auto &item : _items)
    {
        item->format(ss, msg);
    }
    return ss.str();
}

bool LogSys::Formatter::parsePattern()
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

LogSys::FormatItem::ptr LogSys::Formatter::createItem(const std::string &key, const std::string &val)
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