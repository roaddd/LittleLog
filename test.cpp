#include <iostream>
#include "LittleLog.hpp"

int main()
{
    littlelog::init("/tmp/","log",1);
    LOG_INFO<<"Hello"<<888;
    LOG_INFO<<"第一条测试日志\n";
    LOG_INFO<<"第二条测试日志：infojfosdfosdjisjf";
    return 0;
}
