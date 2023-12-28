#include"publishAndSubcribe.hpp"
using namespace redis;

Subscriber::Subscriber(ConnectionPool &pool):_subscriber(pool.fetch()),_stop(0),_pool(&pool){
    assert(!(*_subscriber).broken())
}

void Subscriber::subscribe(const string &channel){
    if(_stop){
        throw ReplyError("subscriber has been stopped");
    }
    assert(!_subscriber->broken());
    auto conn = *_subscriber;
    conn.send("subscribe %b", channel.c_str());
    Connection::ReplyUPtr r =conn.recv();
    auto reply = *r;
    if(!reply || reply->type != REDIS_REPLY_ARRAY || reply->elements != 3){
        throw ReplyError("subscribe failed");
    }
    _listen_thread = std::thread(&Subscriber::listen_func,this);
    return;
}


void Subscriber::unsubscribe(const string &channel){
    auto conn = *_subscriber;
    assert(_listen_thread.joinable());
    conn.send("unsubscribe %b", channel.c_str());
    Connection::ReplyUPtr r =conn.recv();
    stop();
    return;
}

void Subscriber::listen_func(){
    auto conn = *_subscriber;
    redisReply* r = nullptr;
    while(!_stop &&redisGetReply(conn._context(),(void**)&r)==REDIS_OK ){
        if(r!=nullptr && r->type == REDIS_REPLY_ARRAY && r->elements == 3){
            redisReply* subscribeReply = r->element[0];
            redisReply* channelReply = r->element[1];
            redisReply* messageReply = r->element[2];
            _callback(channelReply->str, messageReply->str);
        }
        freeReplyObject(r);
    }
    return;
}

Publisher::Publisher(ConnectionPool &pool):_publisher(pool.fetch()),_pool(&pool){
    assert(!(*_publisher).broken());
}

void Publisher::publish(const string &channel, const string &message){
    _publisher->send("publish %b %b", channel.c_str(), message.c_str());
    Connection::ReplyUPtr r = _publisher->recv();
    return;
}

