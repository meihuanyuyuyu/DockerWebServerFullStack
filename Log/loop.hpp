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
        AsyncLooper(const std::function<void(Buffer &)> &cb, AsyncType loop_type = AsyncType::ASYNC_SAFE):_stop(false),_loop_type(loop_type),_thread(std::thread(&AsyncLooper::run,this)),_call_back(cb){};
        ~AsyncLooper(){
            stop();
        };
        
        void stop(){
            _stop = true;
            _cond_con.notify_all();
            _thread.join();
        }

        void push(const char *data, size_t len){
            if(_stop==true){
                return;
            }
            {
                std::unique_lock<std::mutex> lock(_lock);
                if(_loop_type==AsyncType::ASYNC_SAFE){
                    _cond_pro.wait(lock,[&](){return _producer.WriteAbleSize()>=len;});
                }
                _producer.Push(data,len);

            }
            _cond_con.notify_one();
        }

        private:
        void run(){
            while(1){
                {
                    std::unique_lock<std::mutex> lock(_lock);
                    if(_stop && _producer.Empty()){
                        break;
                    }
                    _cond_con.wait(lock,[&](){return _stop && !_producer.Empty();});
                    _consumer.Swap(_producer);

                    if(_loop_type==AsyncType::ASYNC_SAFE){
                        _cond_pro.notify_one();
                    }
                }
                _call_back(_consumer);
                _consumer.Reset();
            }
        };
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