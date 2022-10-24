#include <iostream>
#include "LittleLog.hpp"

int main()
{
    using namespace::littlelog;
    LogLine *ll =new LogLine(LogLevel::INFO,"D:\\Github\\c-learning\\NanoLog11\\Log11\\Log11","tmp.txt","None",0);
    *ll<<"Hello LittleLog";
    ll->stringify(std::cout);
    return 0;
}
