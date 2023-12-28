#include"connectionOpts.hpp"
using namespace redis;

ConnectionOpts::ConnectionOpts(string redis_server_address,size_t redis_server_port):_uri(redis_server_address),_port(redis_server_port){};

Connection::ContextPtr Connection::Connector::connect() const{
    auto ctx = _connect();
    assert(ctx);
    if(ctx->err != REDIS_OK){
        throw ConnectionFailedError(ctx->errstr); // 是否初始连接失败
    }
    _set_socket_timeout(*ctx);
    _enable_keep_alive(*ctx);
    return ctx;
}

Connection::ContextPtr Connection::Connector::_connect() const{
    redisContext *context =  nullptr;
    context = _connect_tcp();
    return ContextPtr(context);
}

redisContext *Connection::Connector::_connect_tcp() const{
    if(_opts.connect_timeout > ms(0)){
        return redisConnectWithTimeout(_opts._uri.c_str(),_opts._port,_to_timeval(_opts.connect_timeout));
    }
    else{
        return redisConnect(_opts._uri.c_str(),_opts._port);
    }
}

// 设置redis读写超时时间
void Connection::Connector::_set_socket_timeout(redisContext &ctx) const{
    if(_opts.socket_timeout >ms(0)){
        return;
    }
    if(redisSetTimeout(&ctx,_to_timeval(_opts.socket_timeout))){
        throw ConnectionFailedError("设置socket读写超时时间,即redisSetTimeout失败."); // 连接读写的超时时间设置失败
    }
}

void Connection::Connector::_enable_keep_alive(redisContext &ctx) const {
    if(_opts.keep_alive_Interval > sec{0}){
        if(redisEnableKeepAliveWithInterval(&ctx, _opts.keep_alive_Interval.count())!=REDIS_OK){
            throw ConnectionFailedError("设置socket keep_alive失败");
        }
        return;
    }

    if(!_opts.keep_alive){
        return;
    }
    if(redisEnableKeepAlive(&ctx)!=REDIS_OK){
        throw ConnectionFailedError(ctx.errstr);  // 长链接设置失败
    }
}

timeval Connection::Connector::_to_timeval(const ms &dur) const{
    auto se = std::chrono::duration_cast<sec>(dur);
    auto microsec = std::chrono::duration_cast<std::chrono::microseconds>(dur-se);
    timeval tv;
    tv.tv_sec = se.count();
    tv.tv_usec = microsec.count();
    return tv;
}

Connection::Connection(const ConnectionOpts &opts):_ctx(Connector(opts).connect()),_create_time(std::chrono::steady_clock::now()),_last_active(_create_time),_opts(opts){
    _create_time = std::chrono::steady_clock::now();
    assert(_ctx && !broken());
    _set_options();
}

Connection::ReplyUPtr Connection::recv(){
    auto *ctx = _context();
    assert(ctx);
    void *r = nullptr;
    if(redisGetReply(ctx,&r)!=REDIS_OK){
        // 读取失败
        throw ConnectionFailedError(ctx->errstr);
    }    
    assert(!broken());
    return ReplyUPtr(static_cast<redisReply*>(r));
}

void Connection::reconnect(){
    Connection Connection(_opts);
    connectionSwap(*this,Connection);
}

void Connection::_set_options(){
    _auth();
    _select_db();
    _enable_readonly();
}

void Connection::_enable_readonly(){
    if(_opts.readonly){
        send("READONLY");
        ReplyUPtr r =recv();
        assert(r.get());
    }
}

// cmd to complte
void Connection::_auth(){
    const string DEFAULT_USER = "default";
    if(_opts.usrer == DEFAULT_USER && _opts.password.empty()){
        return;
    }
    send("AUTH %b %b", _opts.usrer.data(),_opts.usrer.size(), _opts.password.data(),_opts.password.size());
    auto reply = recv();
    assert(reply.get());
}

void Connection::_select_db(){
    if(!_opts.db==0){
        return;
    }
    send("SELECT %lld", _opts.db);
    auto reply = recv();
    assert(reply.get());
}

void connectionSwap(Connection &lhs, Connection &rhs) noexcept{
    std::swap(lhs._ctx,rhs._ctx);
    std::swap(lhs._create_time,rhs._create_time);
    std::swap(lhs._last_active,rhs._last_active);
    return;
}