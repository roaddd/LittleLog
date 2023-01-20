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

        Buffer():buffer(static_cast<Item*>(std::malloc(sz*sizeof(Item))))
        {
            for(int i=0;i<=sz;i++)
            {
                write_state[i].store(0,std::memory_order_relaxed);
            }
        }

        ~Buffer()
        {
            unsigned int write_count=write_state[sz].load();
            for(int i=0;i<write_count;i++)
            {
                buffer[i].~Item();
            }
            std::free(buffer);
        }

        bool push(LogLine&& lg,const unsigned int new_idx)
        {
            new (&buffer[new_idx]) Item(std::move(lg));
            write_state[new_idx].store(1,std::memory_order_release);
            //----------------------------------------
            //std::cout<<"push"<<write_state[new_idx].load()<<std::endl;
            return write_state[sz].fetch_add(1,std::memory_order_acquire)+1==sz;
        }

        bool try_pop(LogLine& lg,const unsigned int read_idx)
        {
            unsigned int state=write_state[read_idx].load(std::memory_order_acquire);
            if(!state)return false;
            lg=std::move(buffer[read_idx].lg);
            /*----------------------------------*/
            lg.stringify(std::cout);
            return true;
        }

        Buffer(const Buffer&)=delete;
        Buffer& operator=(const Buffer&)=delete;
    private:
        Item* buffer;
        std::atomic<unsigned int> write_state[sz+1];
    };

}

#endif 