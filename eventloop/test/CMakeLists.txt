cmake_minimum_required(VERSION 3.2)

project(sync_server)

if(NOT MUDUO_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "release")
endif()

#if(NOT TRAPPIST-1_PATH)
#    set (TRAPPIST-1_PATH "/home/$ENV{USER}/work/work_cpp/TRAPPIST-1")
#endif()

set(SYNC_SERVER_NET_PATH "/home/$ENV{USER}/Server/src/qb")

set(CXX_FLAGS
 -std=c++0x                     #TODO c+11
 -g                             #TODO 为了gdb
 -Wall                          #TODO 显示全部的警告
 -Wextra                        #TODO 打印一些额外的警告
 -Werror                        #TODO 警告当作错误
 -Wconversion                   #TODO 当隐式类型转换可能改变值时给出警告
 -Wno-unused-parameter          #TODO 未使用参数会被警告
 -Wold-style-cast               #TODO 程序使用 C风格的类型转换时给出警告
 -Woverloaded-virtual           #TODO 重载虚函数名时给出警告
 -Wpointer-arith                #TODO 当在算术表达式中使用函数指针时给出警告
 -Wshadow                       #TODO 当一个局部变量掩盖了另一个局部变量时给出警告
 -Wwrite-strings                #TODO 在 C++中，非零值表示将字面字符串转换为‘char*’时给出警告。在 C中，给出相似的警告，但这种类型转换是符合ISO C 标准的
 -march=native                  #TODO gcc会自动检测你的CPU支持的指令集

 -rdynamic                      #TODO 给backtrace_symbols使用 指示连接器把所有符号（而不仅仅只是程序已使用到的外部符号）都添加到动态符号表（即.dynsym表）里，
                                #     以便那些通过 dlopen() 或 backtrace() （这一系列函数使用.dynsym表内符号）这样的函数使用
)

string (REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-o0")                                        #TODO 不做优化
set(CMAKE_CXX_FLAGS_RELEASE "-o2 -finline-limit=1000 -DNDEBUG")         #TODO 做优化 最大内联函数的长度被设置 关闭断言
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)                   #TODO 可执行的位置
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)                      #TODO 库输出的位置

#find_path(TRAPPIST-1_INCLUDE_DIR muduo "${TRAPPIST-1_PATH}/include")         #TODO 包含muduo文件的目录 保存在变量Muduo_INCLUDE_DIR
#find_path(TRAPPIST-1_LIBRARY_DIR libmuduo_net.a "${MUDUO_PATH}/lib")   #TODO 包含libmuduo_net.a的目录 保存在变量Muduo_LIBARAY_DIR
#set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${Muduo_LIBRARY_DIR})      #TODO 设置库地址
#message(STATUS ${TRAPPIST-1_INCLUDE_DIR})
#message(STATUS ${Muduo_LIBRARY_DIR})

#include_directories(${TRAPPIST-1_INCLUDE_DIR})                               #TODO 设置头文件目录
#find_library(muduo_base muduo_base)                                     #TODO 在lib目录查找muduo_base库,保存在变量muduo_base
#find_library(muduo_net muduo_net)                                       #TODO 在lib目录查找muduo_net库,保存在变量muduo_net

#message(STATUS ${muduo_base})                                           
#message(STATUS ${muduo_net})
#message(${TRAPPIST-1_PATH})
#message(${PROJECT_SOURCE_DIR})

#include_directories(${TRAPPIST-1_PATH}/..)
include_directories(${SYNC_SERVER_NET_PATH})
include_directories(${PROJECT_SOURCE_DIR})                              #TODO 


add_executable(net_test ./net_test.cpp ../EPoller.cpp ../Channel.cpp ../IPoller.cpp ../EventLoop.cpp)

#add_executable(net_test ./net_test.cpp ${SYNC_SERVER_NET_PATH}/sync_server/net/EventLoop.cpp)
#target_link_libraries(dowload ${muduo_base})
#target_link_libraries(dowload ${muduo_net})
#target_link_libraries(dowload pthread rt)

