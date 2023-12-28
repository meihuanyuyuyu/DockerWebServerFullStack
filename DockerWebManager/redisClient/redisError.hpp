#ifndef __REDIS_ERROR__H
#define __REDIS_ERROR__H

#include<exception>
#include<string>

namespace redis{
    class ConnectionFailedError:public std::exception{
        public:
            ConnectionFailedError(const std::string msg):_msg(msg){};
            const char *what() const noexcept override{
                return _msg.c_str();
            }
        private:
            std::string _msg;
    };

    class ConnectionSendError:public std::exception{
        public:
            ConnectionSendError(const std::string msg):_msg(msg){};
            const char *what() const noexcept override{
                return _msg.c_str();
            }
        private:
            std::string _msg;
    };
    class ConnectionPoolFetchError:public std::exception{
        public:
            ConnectionPoolFetchError(const std::string msg):_msg(msg){};
            const char *what() const noexcept override{
                return _msg.c_str();
            }
        private:
            std::string _msg;
    };

    class ReplyError:public std::exception{
        public:
            ReplyError(const std::string msg):_msg(msg){};
            const char *what() const noexcept override{
                return _msg.c_str();
            }
        private:
            std::string _msg;
    };



}
#endif