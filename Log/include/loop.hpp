#ifndef __M_LOOPER_H__
#define __M_LOOPER_H__


#include"buffer.hpp"
#include<mutex>
#include<condition_variable>
#include<thread>
#include<functional>
#include<memory>
#include<atomic>


namespace LogSys{
    enum class AsyncType
    {
        ASYNC_SAFE,  // 安全状态，表示缓冲区满了就阻塞，避免资源耗尽
        ASYNC_UNSAFE // 不考虑资源耗尽问题，无限扩容，用于测试
    };
    class AsyncLooper{
        public:
        using ptr = std::shared_ptr<AsyncLooper>;
        AsyncLooper(const std::function<void(Buffer &)> &cb, AsyncType loop_type = AsyncType::ASYNC_SAFE);
        ~AsyncLooper();
        void stop();
        void push(const char *data, size_t len);

        private:
        void run();
        private:
            std::function<void(Buffer &)> _call_back;
        private:
            std::atomic<bool> _stop;
            std::mutex _lock;
            std::condition_variable _cond_pro;
            std::condition_variable _cond_con;
            std::thread _thread;
            AsyncType _loop_type;
            Buffer _producer;
            Buffer _consumer;
    };
}

#endif