cmake_minimum_required(VERSION 3.5)

project(LogSystem)

#是否开启debug模式 
set(CMAKE_BUILD_TYPE "Debug")


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(./include) #添加头文件目录
aux_source_directory(./src DIR_SRCS) #添加源文件目录

add_library(log SHARED ${DIR_SRCS}) #生成动态库

add_executable(test test.cc) #生成可执行文件

target_link_libraries(test log pthread) #链接动态库