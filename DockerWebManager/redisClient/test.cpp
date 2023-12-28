#include"connectionPool.hpp"
#include"connectionOpts.hpp"
#include"redisError.hpp"
#include<iostream>
using namespace redis;
using namespace std;

int main(){
    ConnectionPool pool = ConnectionPool(ConnectionPoolOpts(),ConnectionOpts("localhost",6379));
    Connection conn = pool.fetch();
    conn.send("set %b %b","hello", 5 ,"world", 5);
    Connection::ReplyUPtr r = conn.recv();
    cout<<r->str<<endl;
    return 0;
}