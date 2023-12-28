#include"redisDistributeLock.hpp"
using namespace redis;


RedisLock::RedisLock():_conn(nullptr){}

RedisLock::RedisLock(Connection &conn):_conn(conn){};

bool lock(RedisLock &lock){
    if(lock._conn.broken()){
        return false;
    }
    auto randval = std::rand();
    auto key = lock.key;
    auto conn = lock._conn;
    lock._conn.send("SET %s %d NX PX %d", key.c_str(), randval, lock.expire);
    Connection::ReplyUPtr r =lock._conn.recv();
    auto *reply = r.get();
    if(reply == nullptr){
        return false;
    }
    if(reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str, "OK") == 0){
        return true;
    }
    return false;
}

bool unlock(RedisLock &lock){
    if(lock._conn.broken()){
        return false;
    }
    auto key = lock.key;
    auto conn = lock._conn;
    lock._conn.send("EVAL %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s", 
    "if redis.call(\"get\",KEYS[1]) == ARGV[1] then", 
    "return redis.call(\"del\",KEYS[1])", 
    "else", 
    "return 0", 
    "end", 
    "1", 
    key.c_str(), 
    "1", 
    std::to_string(lock.randval).c_str(), 
    "2", 
    "NX", 
    "3", 
    "PX", 
    "4", 
    std::to_string(lock.expire).c_str());
    Connection::ReplyUPtr r =lock._conn.recv();
    auto *reply = r.get();
    if(reply == nullptr){
        return false;
    }
    if(reply->type == REDIS_REPLY_INTEGER && reply->integer == 1){
        return true;
    }
    return false;
}




