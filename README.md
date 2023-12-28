# DockerWebServerFullStack
A web server to manager the docker images, containers from different devices. Moreover, it is utilized to accomplish the GPU resource managering.


## 同步/异步双缓冲日志系统

### 解决难点：
每个线程阻塞调用打印方法：高并发情况下，资源阻塞没有及时释放；
改进第一步：加入额外的打印线程，将日志缓存合并后日志线程打印，解决阻塞问题。
还存在的问题，缓存存在多线程竞争关系，日志线程写入日志的时候上锁时间过长，并发量不高。
改进第二部：日志线程实现双缓冲，使得上锁时间仅有缓存交换的开销时间。

### 技术栈：
设计模式：
（饿汉）单例模式，在函数内通过静态局部变量实现一次初始化。
建造者模式，建造者通过传入实际的子类对象，使用虚函数进行子类的构建方法进行构建。
工厂模式，以函数create的方式创建出不同的子类对象。

语法特性：
智能指针。
C++虚函数动态绑定，用容器存储基类类型指针，动态地使用派生类打印功能。
### 测试1：
二进制文件读写快于文本文件读写，C文件指针操作快于C++流式读写
都存在缓冲区。

```CPP
class BuildBase{
    public:
    virtual void getRoof()=0;
    virtual void getWall()=0;
    Base *build(Base *mode){
        mode->getRoof();
        mode->getWall();
        return mode;
    }
    private:
    Base *base;
};
class Mode1:public BuildBase{
    public:
    virtual void getRoof() override {std::cout<<"1";}
    virtual void getWall() override {std::cout<<"1";}
};
class Mode2:public BuildBase{
    public:
    virtual void getRoof() override {std::cout<<"2";}
    virtual void getWall() override {std::cout<<"2";}
};
BuildBase a;
BuildBase *p = new Mode1;
a.build(p);
```


# Async vs Sync:

对于多线程(5000)的读写打印请求，提升20%的运行时间。
分析：1. 异步执行能够加快线程释放速度，避免阻塞于读写 2. 数据延迟回写，减少IO调用次数。
