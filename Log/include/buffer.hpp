/* 实现异步日志缓冲区 */
#ifndef __M_BUFFER_H__
#define __M_BUFFER_H__
#include <vector>
#include <cassert>
namespace LogSys
{
#define DEFAULT_BUFFER_SIZE (1024 * 1024 * 1)
#define THRESHOLD_BUFFER_SIZE (1024 * 1024 * 8)
#define INCREMENT_BUFFER_SIZE (1024 * 1024 * 1)
    class Buffer
    {
    public:
        Buffer();
        // 向缓冲区写数据
        void Push(const char *data, size_t len);
        // 返回可读数据的起始地址
        const char *Begin();
        // 返回可读写数据长度
        size_t ReadAbleSize() const;
        size_t WriteAbleSize() const;
        // 对读指针进行偏移
        void MoveReader(size_t len);
        // 重置读写位置初始化缓冲区
        void Reset();
        // 对buffer实现交换
        void Swap(Buffer &buffer);
        // 判断缓冲区是否为空
        bool Empty() const;

    private:
        // 对写指针进行偏移
        void MoveWriter(size_t len);
        // 对空间扩容
        void ensureEnoughSize(size_t len);
    private:
        std::vector<char> _buffer;
        size_t _reader_index; // 当前可读数据指针 -- 本质是下标
        size_t _writer_index; // 当前可写数据指针
    };
}
#endif