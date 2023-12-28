# 基于redis++源码阅读

## redis客户端连接池

属性


## 发布订阅功能
作用：通信

客户端observer，维护一个循环阻塞线程，接受通道消息，然后调用回调函数
发布者，调用发布函数，将消息发送到通道

流水线：redisAppendCommand 将指令存储到context的缓冲区。在redisGetReply的时候，从context读缓冲区读取一个数据，没有reply的话会阻塞，并将发送缓冲区内所有命令从socket缓冲区中发送出去。