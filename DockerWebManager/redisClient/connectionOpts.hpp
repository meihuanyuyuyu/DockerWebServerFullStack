#ifndef __CONNECTION_OPTS_HPP
#define __CONNECTION_OPTS_HPP

#include"redisError.hpp"
#include <chrono>
#include <cassert>
#include <memory>
#include <hiredis.h>

using string = std::string;
using ms = std::chrono::milliseconds;
using sec = std::chrono::seconds;
using time_p = std::chrono::time_point<std::chrono::steady_clock>;

namespace redis{
    class ConnectionOpts
    {
    public:
        ConnectionOpts() = default;

        ConnectionOpts(string redis_server_address, size_t redis_server_port);
        ConnectionOpts(const ConnectionOpts &opts) = default;
        ConnectionOpts &operator=(const ConnectionOpts &opts) = default;
        
        ConnectionOpts(ConnectionOpts &&opts) = default;
        ConnectionOpts &operator=(ConnectionOpts && opts)=default;
        ~ConnectionOpts() = default;

        string _uri;
        string usrer = "default";
        string password;
        int _port = 6379;
        string path;
        bool keep_alive = true;
        bool readonly = false;  
        size_t db = 0;
        std::chrono::seconds keep_alive_Interval = std::chrono::seconds(0);
        std::chrono::milliseconds connect_timeout = std::chrono::milliseconds(0);
        std::chrono::milliseconds socket_timeout = std::chrono::milliseconds(0);
    };

    class Connection
    {
    public:
        explicit Connection(const ConnectionOpts &opts);

        Connection(const Connection &conn) = delete;
        Connection &operator=(const Connection &conn) = delete;

        Connection(Connection &&conn) = default;
        Connection &operator=(Connection &&conn) = default;
        ~Connection()=default;

        friend void connectionSwap(Connection &lhs, Connection &rhs) noexcept;

        bool broken() const noexcept{
            return !_ctx || _ctx->err != REDIS_OK;
        }

        void reset() noexcept{
            assert(_ctx);
            _ctx->err = REDIS_OK;
        }

        void invalidate() noexcept{
            assert(_ctx);
            _ctx->err = REDIS_ERR;
        }
        void reconnect();
        auto create_time() const -> time_p{
            return _create_time;
        }
        auto last_active() const -> time_p{
            return _last_active;
        }

        template<typename ...Args>
        void send(const string &format, Args &&...args){
            auto ctx = _context();
            assert(ctx!=nullptr);
            if(redisAppendCommand(ctx, format.c_str() ,std::forward<Args>(args)...)!=REDIS_OK){
                throw ConnectionSendError("send error"); // 发送命令失败
            }
            assert(!broken());
        };

        const ConnectionOpts &get_opt() const{
            return _opts;
        }


        struct ContextDeleter
        {
            void operator()(redisContext *ctx) const {
                if(!ctx){
                    redisFree(ctx);
                }
            }
        };

        struct ReplyDeleter{
            void operator()(redisReply *reply) const{
                if(!reply){
                    freeReplyObject(reply);
                }
            }
        };
        using ContextPtr = std::unique_ptr<redisContext, ContextDeleter>;
        using ReplyUPtr = std::unique_ptr<redisReply,ReplyDeleter>;

        ReplyUPtr recv();

    private:

        friend class ConnectionPool;
        // Connector 根据opts来返回连接好的rediscontext智能对象。
        class Connector{
            public:
            explicit Connector(const ConnectionOpts &opts):_opts(opts){};
            Connection::ContextPtr connect() const;
            private:
            redisContext *_connect_tcp() const;
            Connection::ContextPtr _connect() const;
            void _enable_keep_alive(redisContext &ctx) const;
            void _set_socket_timeout(redisContext &ctx) const;
            timeval _to_timeval(const ms &dur) const;
            const ConnectionOpts &_opts;
        };
    
        void _set_options();
        void _auth();
        void _select_db();
        void _enable_readonly();
        redisContext *_context();

    private:
        ContextPtr _ctx;
        time_p _create_time{};
        time_p _last_active{};
        const ConnectionOpts &_opts;
    };

    using ConnectionPtr = std::shared_ptr<Connection>;

    //每次取出redis上下文用于发送命令，据此更新最后活跃时间
    inline redisContext* Connection::_context(){
        _last_active = std::chrono::steady_clock::now();
        return _ctx.get();
    };


}
#endif