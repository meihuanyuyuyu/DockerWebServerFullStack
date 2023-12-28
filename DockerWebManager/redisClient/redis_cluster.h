#ifndef _REDISPLUSPLUS_REDIS_CLUSTER_H
#define _REDISPLUSPLUS_REDIS_CLUSTER_H

#include <string>
#include<chrono>
#include<initializer_list>
#include<tuple>
#include<memory>
#include"connectionPool.hpp"


'''
redis++客户端 core 功能：
1. pipeline
2. transaction
3. publish&subcriber
4. redis数据库操作
'''

namespace redis{
    class Redis{
        public:
        // redis pool
        explicit Redis(const ConnectionOpts &opts, const ConnectionPoolOpts &pool_opts={}):
            _pool(std::make_shared<ConnectionPool>(pool_opts,opts)){};
        //redis istance with uri
        explicit Redis(const std::string &uri):Redis(Uri(uri)){};

        Redis(const Redis &redis) = delete;
        Redis& operator=(const Redis &) = delete;
        Redis(Redis &&) = default;
        Redis &operator=(Redis &&) = default;

        // pipeline to reduce RTT and speed up redis queries.
        Pipeline pipeline(bool new_connection=true);
        // transaction
        Transaction transaction(bool piped=false,bool new_connection=true);
        // publish& subcriber
        Subscriber subscriber();

        //数据操作：
        // key
        bool set(const std::String &key, const std::string &value, const std::chrono::milliseconds &expire = std::chrono::milliseconds(0), bool exist = false, bool new_connection = true);
        bool get(const std::string &key, std::string &value, bool new_connection = true);

        // list
        long long lpush(const std::string &key, const std::string &val);
        template<typename Input>
        long long lpush(const std::string &key, Input first, Input last);
        template<typename T>
        long long lpush(const std::string &key, std::initializer_list<T> ilist);
        
        //hash command:
        bool hset(const std::string &key, const std::string &filed, const std::string &value);
        bool hset(const std::string &key, std::pair<std::string,std::string> &&field_value);
        bool hmset(const std::string &key, std::unordered_map::iterator<std::string> strat,std::unordered_map::iterator<std::string> end);
        bool hgetall(const std::string &key, std::insert_iterator<std::unordered_map<std::string,std::string>> iter);
        
        
        // set:
        long long sadd(const std::string &key, const std::string &member);
        template<typename Input>
        long long sadd(const std::string &key, Input first, Input last);
        template<typename T>
        long long sadd(const std::string &key, std::initializer_list<T> ilist){
            return sadd(key,ilist.begin(),ilist.end());
        }
        template< typename Output>
        void smembers(const std::string &key, Output output);

        // sorted set
        long long zadd(const std::string &key, const std::string &member, double score);
        template<typename Input>
        long long zadd(const std::string &key, Input first, Input last);
        template<typename T>
        long long zadd(const std::string &key, std::initializer_list<T> ilist){
            return zadd(key,ilist.begin(),ilist.end());
        };
        template<typename Interval, typename Output>
        void zrangebyscore(const std::string &key,const Interval &interval, Output Output);
        
        private:
        std::shared_ptr<ConnectionPool> _pool;
    }


}



#endif