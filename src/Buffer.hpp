#ifndef __BUFFER_HPP__
#define __BUFFER_HPP__

#include <atomic>
#include "LittleLog.hpp"

namespace littlelog
{
/**
     * @brief 日志信息的缓冲区，每个缓冲区大小为8MB，每个日志信息占用的空间为256字节，因此每个
     *        缓冲区可存放的日志信息的数量为32768个
     * 
     */
    class Buffer
    {
    public:
        struct Item
        {
            Item(LogLine&&lg):lg(std::move(lg)){};
            char padding[256-sizeof(LogLine)];
            LogLine lg;
        };
        static constexpr const size_t sz=32768;//8MB/256B=32768

        Buffer();

        ~Buffer();
        
        bool push(LogLine&& lg,const unsigned int new_idx);

        bool try_pop(LogLine& lg,const unsigned int read_idx);

        Buffer(const Buffer&)=delete;
        Buffer& operator=(const Buffer&)=delete;
    private:
        Item* buffer;
        std::atomic<unsigned int> write_state[sz+1];
    };

}

#endif 