#include"loop.hpp"

using namespace LogSys;

AsyncLooper::AsyncLooper(const std::function<void(Buffer &)> &cb, AsyncType loop_type):_stop(false),_loop_type(loop_type),_thread(std::thread(&AsyncLooper::run,this)),_call_back(cb){};
AsyncLooper::~AsyncLooper(){
    stop();
};

void AsyncLooper::stop(){
    _stop = true;
    _cond_con.notify_all();
    _thread.join();
}

void AsyncLooper::push(const char *data, size_t len){
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

void AsyncLooper::run(){
    while(1){
        std::unique_lock<std::mutex> lock(_lock);
        if(_stop && _producer.Empty()){
            break;
        }
        _cond_con.wait(lock,[&](){return _stop || !_producer.Empty();});
        _consumer.Swap(_producer);
    }
    _call_back(_consumer);
    _consumer.Reset();
}


