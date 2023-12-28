#ifndef _PUBLISHANDSUBCRIBE_HPP_
#define _PUBLISHANDSUBCRIBE_HPP_

#include<hiredis>
#include<thread>
#include<functional>
#include"connectionOpts.hpp"
#include"connectionPool.hpp"

namespace redis{
    class Subscriber{    
        public:
        //析构函数
        Subscriber()=default;
        Subscriber(const Subscriber& obj) = delete;
        Subscriber operator=(const Subscriber& obj)=delete;
        Subscriber(ConnectionPool &pool);
        ~Subscriber(){
            if(!_stop){
                unsubscribe(_channel);
            }
            _pool->release(std::move(_subscriber));
            if(_listen_thread.joinable()){
                _listen_thread.join();
            }
        };
        void subscribe(const string &channel);
        void unsubscribe(const string &channel);
        void stop(){
            _stop = 1;
        }
        void listen_func();
        
        private:
        string _channel;
        std::function<void(const string, const string)> _callback;
        ConnectionPtr _subscriber;
        ConnectionPool* _pool;
        std::thread _listen_thread;
        int _stop{0};


    }
    
    class Publisher{
        public:
        Publisher()=default;
        Publisher(const Publisher& obj) = delete;
        Publisher operator=(const Publisher& obj)=delete;
        Publisher(ConnectionPool &pool);
        ~Publisher(){
            _pool.release(std::move(_publisher));
        };
        void publish(const string &channel, const string &message);
        private:
        int _stop{0};
        ConnectionPtr _publisher;
        ConnectionPool* _pool;
    }


} // namespace redis;

#endif