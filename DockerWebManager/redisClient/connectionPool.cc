#include "connectionPool.hpp"
#include<exception>
using namespace redis;

ConnectionPoolOpts::ConnectionPoolOpts() : max_connection(5), wait_timeout(0), connection_lifetime(0), connection_idle_time(0){};
ConnectionPoolOpts::ConnectionPoolOpts(size_t max_connection, std::chrono::milliseconds wait_timeout, std::chrono::milliseconds connection_lifetime, std::chrono::milliseconds connection_idle_time) : max_connection(max_connection), wait_timeout(wait_timeout), connection_lifetime(connection_lifetime), connection_idle_time(connection_idle_time){};

ConnectionPool::ConnectionPool(const ConnectionPoolOpts &pool_opts, const ConnectionOpts &connection_opts) : _popts(pool_opts), _copts(connection_opts){
    if(_popts.max_connection == 0){
        throw "max_connection must be greater than 0";
    }
};

ConnectionPool::ConnectionPool(ConnectionPool &&tmp){
    std::lock_guard<std::mutex> lock(tmp._mutex);
    _move(std::move(tmp));
};

ConnectionPool &ConnectionPool::operator=(ConnectionPool &&tmp){
    if(this != &tmp){
        std::lock_guard<std::mutex> lock(tmp._mutex);
        std::lock_guard<std::mutex> lock2(_mutex);

        _move(std::move(tmp));
    }
    return *this;
}

Connection ConnectionPool::fetch(){
    std::unique_lock<std::mutex> lock(_mutex);
    auto connection = _fetch(lock);
    auto conn_lifetime = _popts.connection_lifetime;
    auto conn_idle_time = _popts.connection_idle_time;
    lock.unlock();


    if(_need_reconnect(connection,conn_lifetime,conn_idle_time)){
        try{
            connection.reconnect();
        }
        catch(const ConnectionFailedError &e){
            release(std::move(connection));
            throw e;
        }
    }
    return connection;
}

ConnectionOpts ConnectionPool::get_connection_opts(){
    std::lock_guard<std::mutex> lock(_mutex);
    return _copts;
}

void ConnectionPool::release(Connection conn){
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _pool.push_back(std::move(conn));
    }
    _cv.notify_one();
}

Connection ConnectionPool::create(){
    std::unique_lock<std::mutex> lock(_mutex);
    auto ops = _copts;
    return Connection(ops);
}

ConnectionPool ConnectionPool::clone(){
    std::lock_guard<std::mutex> lock(_mutex);
    return ConnectionPool(_popts,_copts);
}

void ConnectionPool::_move(ConnectionPool &&tmp){
    _copts = std::move(tmp._copts);
    _popts = std::move(tmp._popts);
    _pool = std::move(tmp._pool);
    _used_connection = tmp._used_connection;
}

Connection ConnectionPool::_fetch(std::unique_lock<std::mutex> &lock){
    if(_pool.empty()){
        if(_used_connection == _popts.max_connection){
            _wait_for_connection(lock);
        }
        else{
            ++_used_connection;
            return Connection(_copts);
        }
    }
    return _fetch();
}

Connection ConnectionPool::_fetch(){
    assert(!_pool.empty());

    auto connection = std::move(_pool.front());
    _pool.pop_front();
    return connection;
}

void ConnectionPool::_wait_for_connection(std::unique_lock<std::mutex> &lock){
    auto timeout = _popts.wait_timeout;
    if(timeout > std::chrono::milliseconds(0)){
        if(!_cv.wait_for(lock,timeout, [this](){return !(this->_pool.empty());})){
            throw ConnectionPoolFetchError("Failed to fetch connection in "+ std::to_string(timeout.count()) + " milliseconds");
        }
    }
    else{
        _cv.wait(lock,[this](){return !(this->_pool.empty());});
    }
}

bool ConnectionPool::_need_reconnect(Connection &conn, std::chrono::milliseconds &conn_lifetime,std::chrono::milliseconds &conn_idle_time) const{
    if(conn.broken()) return true;
    auto now = std::chrono::steady_clock::now();

    if(conn_lifetime > std::chrono::milliseconds(0)){
        if(now-conn.create_time()>conn_lifetime){
            return true;
        }
    }
    if(conn_idle_time > std::chrono::milliseconds(0)){
        if(now-conn.last_active()>conn_idle_time){
            return true;
        }
    }
    return false;
}
