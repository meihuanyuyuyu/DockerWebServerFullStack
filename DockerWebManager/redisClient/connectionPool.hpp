#ifndef __CONNECTION_POOL_HPP
#define __CONNECTION_POOL_HPP
#include"connectionOpts.hpp"
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <thread>
#include<list>

namespace redis
{
    // 连接池配置: 最大连接数、连接等待时间、连接线程生命周期、连接空闲时间
    struct ConnectionPoolOpts
    {
        // 0 means no limit;
        ConnectionPoolOpts();
        ConnectionPoolOpts(size_t max_connection, std::chrono::milliseconds wait_timeout, std::chrono::milliseconds connection_lifetime, std::chrono::milliseconds connection_idle_time);
        size_t max_connection;
        std::chrono::milliseconds wait_timeout;
        std::chrono::milliseconds connection_lifetime; 
        std::chrono::milliseconds connection_idle_time;// idle time of connection, if a connection is idle for a long time, it will be closed
    };


    // 连接池：1. 连接池结构与其配置、2. 连接线程配置、3.同步锁与条件变量、
    // 方法：默认构造、移动构造
    class ConnectionPool
    {
    public:
        ConnectionPool() = default;
        explicit ConnectionPool(const ConnectionPoolOpts &pool_opts, const ConnectionOpts &connection_ops); 
        
        ConnectionPool(ConnectionPool && tmp);
        ConnectionPool &operator=(ConnectionPool && tmp);

        //静止拷贝构造函数、赋值运算符
        ConnectionPool(const ConnectionPool &) = delete;
        ConnectionPool &operator=(const ConnectionPool &) = delete;
        ~ConnectionPool()=default;

        //从池获取连接
        Connection fetch();
        //归还连接
        void release(Connection conn);
        //获取连接配置
        ConnectionOpts get_connection_opts();

        
        Connection create();
        ConnectionPool clone();

    private : 
        std::mutex _mutex;
        std::condition_variable _cv;
        ConnectionOpts _copts;
        ConnectionPoolOpts _popts;
        std::list<Connection> _pool;
        size_t _used_connection{0};
    private:
        void _move(ConnectionPool && tmp);
        Connection _create();// sential 尚未实现
        Connection _fetch();
        Connection _fetch(std::unique_lock<std::mutex> &lock);
        void _wait_for_connection(std::unique_lock<std::mutex> &lock);
        bool _need_reconnect(Connection &conn,std::chrono::milliseconds &conn_lifetime,std::chrono::milliseconds &conn_idle_time) const;
        inline void _update_connection_opts(const std::string &host,int port){
            _copts._uri = host;
            _copts._port = port;
        }
        inline bool _role_changed(const ConnectionOpts &opts) const{
            return _copts._uri != opts._uri || _copts._port != opts._port;
        }
    };

} // namespace redis
#endif
