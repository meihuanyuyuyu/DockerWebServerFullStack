#include"redis.hpp"
#include"log.hpp"



void *Redis::connect(){
    _context = redisConnect()
}

Redis::Redis():