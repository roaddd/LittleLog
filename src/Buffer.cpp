#include "Buffer.hpp"


namespace littlelog
{

        Buffer::Buffer():buffer(static_cast<Item*>(std::malloc(sz*sizeof(Item))))
        {
            for(int i=0;i<=sz;i++)
            {
                write_state[i].store(0,std::memory_order_relaxed);
            }
        }

        Buffer::~Buffer()
        {
            unsigned int write_count=write_state[sz].load();
            for(int i=0;i<write_count;i++)
            {
                buffer[i].~Item();
            }
            std::free(buffer);
        }

        bool Buffer::push(LogLine&& lg,const unsigned int new_idx)
        {
            new (&buffer[new_idx]) Item(std::move(lg));
            write_state[new_idx].store(1,std::memory_order_release);
            return write_state[sz].fetch_add(1,std::memory_order_acquire)+1==sz;
        }

        bool Buffer::try_pop(LogLine& lg,const unsigned int read_idx)
        {
            unsigned int state=write_state[read_idx].load(std::memory_order_acquire);
            if(!state)return false;
            lg=std::move(buffer[read_idx].lg);

            #ifdef TERMINAL_DISPLAY
                lg.stringify(std::cout);
            #endif
            
            return true;
        }



}
