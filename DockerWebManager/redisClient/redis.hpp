#ifndef REDIS_HPP

#define REDIS_HPP
#include"hiredis.h"
#include<QHostAddress>
#include<string>

// QStringList DataBase::_user_column{"id", "account", "nickname", "password", "salt", "email", "phone", "photo"};
// QStringList DataBase::_image_column{"id", "showname", "imagename", "init_args", "description"};
// QStringList DataBase::_container_column{"id", "imageid", "userid", "showname", "containername", "portlist", "running"};
// QStringList DataBase::_machine_column{"id", "ip", "gpu", "cpu", "memory", "online"};
// QStringList JsonKeys{"gpu", "cpu", "memory", "disk", "init_args", "portlist"};


class Redis{
    public:
    Redis()=default;
    Redis(QHostAddress redis_server_address, quint16 redis_server_port);
    Redis(std::string __redis_server_address, unsigned short __redis_server_port);
    Redis(const Redis& redis)=delete;
    Redis& operator=(const Redis& redis)=delete;

    ~Redis();
    virtual setcache(const std::string &key, const std::string &value, int ttlSeconds=0)=0;
    virtual getCache(const std::string &key)=0;
    virtual deleteCache(const std::string &key)=0;

    private:
    LogSys::Logger::ptr _logger;
    redisContext *_context;

    private:
    void *connect();
    void checkConnection();
};


class RedisClient:public Redis{
    public:
    RedisClient();
    RedisClient(QHostAddress redis_server_address, quint16 redis_server_port);
    RedisClient(std::string __redis_server_address, unsigned short __redis_server_port);
    RedisClient(const RedisClient& redis);
    RedisClient& operator=(const RedisClient& redis);

    ~RedisClient();

    private:
    LogSys::Logger::ptr _logger;
    redisContext *_context;

    private:
    void *connect();
    void checkConnection();
};

#endif