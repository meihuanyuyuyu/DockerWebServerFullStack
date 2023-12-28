#include"redis_cluster.h"
using namespace redis;

Pipeline Redis::pipeline(bool new_connection){
    return Pipeline(_pool,new_connection);
}

Transaction Redis::transaction(bool piped,bool new_connection){
    if(!_pool){
        throw Error("connection pool is null");
    }
    return Transaction(_pool,piped,new_connection);
}

Subsriber Redis::subscriber(){
    if(!_pool){
        throw Error("connection pool is null");
    }
    return Subscriber(_pool);
}



