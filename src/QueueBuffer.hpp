#ifndef __QUEUEBUFFER_HPP_
#define __QUEUEBUFFER_HPP_

#include "Buffer.hpp"
#include "SpinLock.hpp"
#include <atomic>
#include <queue>

namespace littlelog
{
    /**
     * @brief 日志信息缓冲区的环形队列，用来存放Buffer
     * 
     */
class QueueBuffer
{
public:
    QueueBuffer();

    void push(LogLine&& lg);

    void get_next_read_buffer();

    bool try_pop(LogLine& lg);
    
    void setup_new_buffer();

private:
    //保证数据同步
    //多个线程的消费者共同访问，需要使用原子变量或者加锁
    std::queue<std::unique_ptr<Buffer>> buffers;
    std::atomic<Buffer*> cur_write_buffer;
    std::atomic<int> write_index;
    std::atomic_flag flag;
    //主线程读取的变量，不存在竞争
    Buffer* cur_read_buffer;
    unsigned int read_index;
    
};
}



#endif