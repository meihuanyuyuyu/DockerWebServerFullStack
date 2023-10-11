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
        Buffer()
            : _buffer(DEFAULT_BUFFER_SIZE), _reader_index(0), _writer_index(0)
        {
        }
        // 向缓冲区写数据
        void Push(const char *data, size_t len)
        {
            // 剩余空间不足 - 扩容
            ensureEnoughSize(len);
            // 1. 将数据拷贝到缓冲区
            std::copy(data, data + len, &_buffer[_writer_index]);

            // 2.将写入位置后移
            MoveWriter(len);
        }
        // 返回可读数据的起始地址
        const char *Begin()
        {
            return &_buffer[_reader_index];
        }
        // 返回可读写数据长度
        size_t ReadAbleSize()
        {
            // 当前实现的缓冲区设计思想是双缓冲区，处理完就交换，不存在循环
            return (_writer_index - _reader_index);
        }
        size_t WriteAbleSize()
        {
            // 对于扩容，不存在可写空间大小，因为总是可写
            // 这个接口仅仅针对固定大小缓冲区
            return (_buffer.size() - _writer_index);
        }
        // 对读指针进行偏移
        void MoveReader(size_t len)
        {
            assert(len <= ReadAbleSize());
            _reader_index += len;
        }
        // 重置读写位置初始化缓冲区
        void Reset()
        {
            _writer_index = 0;
            _reader_index = 0;
        }
        // 对buffer实现交换
        void Swap(Buffer &buffer)
        {
            _buffer.swap(buffer._buffer);
            std::swap(_reader_index, buffer._reader_index);
            std::swap(_writer_index, buffer._writer_index);
        }
        // 判断缓冲区是否为空
        bool Empty()
        {
            return (_reader_index == _writer_index);
        }

    private:
        // 对写指针进行偏移
        void MoveWriter(size_t len)
        {
            assert((len + _writer_index) <= _buffer.size());
            _writer_index += len;
        }
        // 对空间扩容
        void ensureEnoughSize(size_t len)
        {
            if (len <= WriteAbleSize())// 不需要扩容
                return;
            size_t new_size = 0;
            if (_buffer.size() < THRESHOLD_BUFFER_SIZE)
            {
                new_size = _buffer.size() * 2 + len;
            }
            else
            {
                new_size = _buffer.size() + INCREMENT_BUFFER_SIZE + len;
            }
            _buffer.resize(new_size);
        }

    private:
        std::vector<char> _buffer;
        size_t _reader_index; // 当前可读数据指针 -- 本质是下标
        size_t _writer_index; // 当前可写数据指针
    };
}
#endif