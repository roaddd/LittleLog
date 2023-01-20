#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include "LittleLog.hpp"
#include <chrono>
#include <thread>


uint64_t timestamp()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}



std::thread::id this_thread_id()
{
    static const thread_local std::thread::id id=std::this_thread::get_id();
    return id;
}
 
#endif
   