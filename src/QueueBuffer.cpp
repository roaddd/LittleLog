#include "QueueBuffer.hpp"
#include <queue>

namespace littlelog
{
    QueueBuffer::QueueBuffer():cur_read_buffer(nullptr),flag(ATOMIC_FLAG_INIT),write_index(0),read_index(0)
    {
        setup_new_buffer();
    }

    void QueueBuffer::push(LogLine&& lg)
    {
        unsigned int next_write=write_index.fetch_add(1,std::memory_order_relaxed);
        if(next_write<Buffer::sz)
        {
            //std::cout<<"write_index"<<write_index<<std::endl;
            if(cur_write_buffer.load(std::memory_order_acquire)->push(std::move(lg),next_write))
                setup_new_buffer();
        }
        else
        {
            while(write_index.load(std::memory_order_acquire)>=Buffer::sz);
            push(std::move(lg));
        }
    }

    void QueueBuffer::get_next_read_buffer()
     {
        SpinLock sp(flag);
        cur_read_buffer=buffers.size()?buffers.front().get():nullptr;
    }

    bool QueueBuffer::try_pop(LogLine& lg)
    {
        if(cur_read_buffer==nullptr)
            get_next_read_buffer();
        if(cur_read_buffer==nullptr)
            return false;
        Buffer* bf=cur_read_buffer;
        //std::cout<<"read_index: "<<read_index<<std::endl;
        if(bool succedd=bf->try_pop(lg,read_index))
        {
            //std::cout<<succedd<<std::endl;
            read_index++;
            if(read_index==Buffer::sz)//该日志缓冲区已读完
            {
                read_index=0;
                cur_read_buffer=nullptr;
                SpinLock sp(flag);
                buffers.pop();
            }
            return true;
        }
        else
            return false;
    }

    void QueueBuffer::setup_new_buffer()
    {
        std::unique_ptr<Buffer> next_buffer(new Buffer());
        cur_write_buffer.store(next_buffer.get(),std::memory_order_release);
        SpinLock sl(flag);
        buffers.push(std::move(next_buffer));
        write_index.store(0,std::memory_order_relaxed);
    }

}