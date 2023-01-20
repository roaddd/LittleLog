#ifndef __LITTLELOGGER_HPP__
#define __LITTLELOGGER_HPP__

#include <string>
#include <atomic>
#include "QueueBuffer.hpp"
#include "Write_to_file.hpp"


namespace littlelog
{
/**
 * @brief 实现了日志系统的后台线程，该线程不断地检查缓冲区队列中是否存在未写入文件的日志；
 *      如有，取出写入文件，若没有，则sleep
 * 
 */
class LittleLogger
{
public:
    LittleLogger(const std::string& dir,const std::string& file,uint32_t roll_size);

    ~LittleLogger();

    void add(LogLine&& lg);

    void work();
    
private:
    enum class State{
        INTI,READY,SHOUTDOWN
    };
    std::atomic<State> state;
    std::unique_ptr<QueueBuffer> log_buffer;
    write_to_file writer;
    std::thread read_thread;
};
}

#endif