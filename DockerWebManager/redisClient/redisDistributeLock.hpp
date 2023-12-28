#ifndef __REDIS_DISTRIBUTE_LOCK_HPP
#define __REDIS_DISTRIBUTE_LOCK_HPP

#include"connectionOpts.hpp"
#include<hiredis-1.2.0/hiredis.h>
#include<string>


namespace redis
{   
    struct RedisLock
    {
        RedisLock();
        RedisLock(Connection &conn);
        /* data */
        std::string key;
        int randval;
        int expire; //ms
        Connection _conn;
        
    };
    bool lock(RedisLock &lock);
    bool unlock(RedisLock &lock);
} // namespace redis

