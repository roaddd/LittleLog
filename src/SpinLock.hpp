#ifndef __SPINKLOCK_HPP__
#define __SPINKLOCK_HPP__

#include <atomic>

/**
 * @brief 原子变量实现的自旋锁
 * 
 */
struct SpinLock
{
    SpinLock(std::atomic_flag& flag):flag(flag)
    {
        while(flag.test_and_set(std::memory_order_acquire));
    }
    ~SpinLock()
    {
        flag.clear(std::memory_order_release);
    }
private:
    std::atomic_flag& flag;
};

#endif