# DockerWebServerFullStack
A web server to manager the docker images, containers from different devices. Moreover, it is utilized to accomplish the GPU resource managering.


## 同步/异步双缓冲日志系统

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