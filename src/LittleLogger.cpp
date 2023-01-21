#include "LittleLogger.hpp"

namespace littlelog
{
    LittleLogger::LittleLogger(const std::string& dir,const std::string& file,uint32_t roll_size):
    state(State::INTI),log_buffer(new QueueBuffer()),writer(dir,file,roll_size),
    read_thread(&LittleLogger::work,this)
    {
        state.store(State::READY,std::memory_order_release);
        //std::cout<<static_cast<uint32_t>(state.load(std::memory_order_acquire))<<std::endl;
    }

    LittleLogger::~LittleLogger()
    {
        state.store(State::SHOUTDOWN);
        read_thread.join();
    }

    void LittleLogger::add(LogLine&& lg)
    {
        //std::cout<<"add"<<std::endl;
        log_buffer->push(std::move(lg));
    }

    void LittleLogger::work()
    {
        //std::cout<<"work"<<std::endl;
        while(state.load(std::memory_order_acquire)==State::INTI)
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        LogLine curLog(LogLevel::INFO,nullptr,nullptr,0);
        //std::cout<<static_cast<uint32_t>(state.load())<<std::endl;
        while(state.load()==State::READY)
        {
            if(log_buffer.get()->try_pop(curLog))
            {
                writer.write(curLog);
            }    
            else
                std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
        while(log_buffer->try_pop(curLog))
            writer.write(curLog);
        //std::cout<<"---work---"<<std::endl;
    }
}