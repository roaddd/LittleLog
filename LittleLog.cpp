#include "LittleLog.hpp"
#include <chrono>
#include <thread>
#include <cstring>
#include <string>
#include <ostream>
#include <queue>
#include <atomic>

namespace littlelog
{
    uint64_t timestamp()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    typedef std::tuple<char,char*,uint32_t,uint64_t,int32_t,int64_t,double,LogLine::string_literal_t> SupportedTypes;

    std::thread::id this_thread_id()
    {
        static const thread_local std::thread::id id=std::this_thread::get_id();
        return id;
    }

    char* LogLine::get_index()
    {
        return !heap_buffer?&stack_buffer[bytes_used]:&(heap_buffer.get())[bytes_used];
    }

    void LogLine::resize_buffer(size_t length)
    {
        if(bytes_used+length<buffer_size)return;
        size_t needs=bytes_used+length;
        if(!heap_buffer)
        {
            buffer_size=std::max(static_cast<size_t>(512),needs);
            heap_buffer.reset(new char[buffer_size]);
            memcpy(heap_buffer.get(),stack_buffer,bytes_used);
            return;
        }
        else
        {
            buffer_size=std::max(static_cast<size_t>(2*buffer_size),needs);
            std::unique_ptr<char[]> new_heap_buffer(new char[buffer_size]);
            memcpy(new_heap_buffer.get(),heap_buffer.get(),bytes_used);
            new_heap_buffer.swap(heap_buffer);
        }
    }

    template<typename T,typename Tuple>
    struct TupleIndex;

    template<typename T,typename...Types>
    struct TupleIndex<T,std::tuple<T,Types...>>
    {
        static const constexpr std::size_t value=0;
    };

    template<typename T,typename U,typename...Types>
    struct TupleIndex<T,std::tuple<U,Types...>>
    {
        static const constexpr std::size_t value=1+TupleIndex<T,std::tuple<Types...>>::value; 
    };

    void LogLine::encode_c_string(const char* arg,size_t length)
    {
        if(!length)return;
        resize_buffer(length);
        char* cur=get_index();
        auto tp=TupleIndex<char*,SupportedTypes>::value;
        *reinterpret_cast<uint8_t*>(cur++)=static_cast<uint8_t>(tp);
        memcpy(cur,arg,length+1);
        cur+=length+2;
    }

    void LogLine::encode(char* arg)
    {
        if(arg!=nullptr)encode_c_string(arg,strlen(arg));
    }

    void LogLine::encode(const char* arg)
    {
        if(arg!=nullptr)encode_c_string(arg,strlen(arg));
    }

    void LogLine::encode(string_literal_t arg)
    {
        
    }

    LogLine::LogLine(LogLevel level,const char* dir,const char* file,const char* function,uint32_t line)
    :bytes_used(0),buffer_size(sizeof(stack_buffer))
    {
        encode<uint64_t> (timestamp());
        encode<std::thread::id> (this_thread_id());
        encode<string_literal_t> (string_literal_t(file));
        encode<string_literal_t> (string_literal_t(function));
        encode<uint32_t> (line);
        encode<LogLevel>(level);
    }

    template<>
    LogLine& LogLine::operator<<(const char* arg)
    {
        encode_c_string(arg,strlen(arg));
    }

    template<>
    LogLine& LogLine::operator<<(char* arg)
    {
        encode_c_string(arg,strlen(arg));
    }

    void format_time(std::ostream& os,uint64_t times)
    {
        std::time_t t=times/1000000;
        auto g=gmtime(&t);
        char b[32];
        strftime(b,32,"Y-%m-%d %T.",g);
        char mic[7];
        sprintf(mic,"%06llu",times%1000000);
        os<<'['<<b<<mic<<']';
    }

    const char* to_string(LogLevel lg)
    {
        switch (lg)
        {
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARN:
            return "WARN";
        case LogLevel::DEBUG:
            return "DEBUG";
        }
        return "";
    }

    void LogLine::stringify(std::ostream& os)
    {
        char* b=!heap_buffer?stack_buffer:heap_buffer.get();
        const char* const end=b+bytes_used;
        uint64_t times=*reinterpret_cast<uint64_t*>(b);
        b+=sizeof(uint64_t);
        std::thread::id threadid=*reinterpret_cast<std::thread::id*>(b);
        b+=sizeof(std::thread::id);
        string_literal_t file=*reinterpret_cast<string_literal_t*>(b);
        b+=sizeof(string_literal_t);
        string_literal_t function=*reinterpret_cast<string_literal_t*>(b);
        b+=sizeof(string_literal_t);
        uint32_t line =*reinterpret_cast<uint32_t*>(b);
        b+=sizeof(uint32_t);
        LogLevel lg=*reinterpret_cast<LogLevel*>(b);
        b+=sizeof(LogLevel);

        //转换成可视化的时间:2022:10:20 20:37:57.666666
        format_time(os,times);
        os  <<'['<<to_string(lg)<<']'<<'['<<threadid<<']'<<'['<<file.s<<':'
            <<function.s<<':'<<line<<']';
        stringify(os,b,end);
        os<<"\n";
        if(lg>=LogLevel::INFO)
            os.flush();
    }

    template<typename Arg>
    char* decode(std::ostream& os,char* b,Arg* dummy)
    {
        Arg arg=*reinterpret_cast<Arg*>(b);
        os<<arg;
        b+=sizeof(Arg);
        return b;
    }

    template<>
    char* decode(std::ostream& os,char* b,LogLine::string_literal_t* dummy)
    {
        LogLine::string_literal_t s=*reinterpret_cast<LogLine::string_literal_t*>(b);
        os<<s.s;
        b+=sizeof(LogLine::string_literal_t);
        return b;
    }

    template<>
    char* decode(std::ostream&os,char* b,char** dummy)
    {
        while(*b!='\0')
        {
            os<<*b;
            b++;
        }
        return ++b;
    }

    void LogLine::stringify(std::ostream& os,char* start,const char* end)
    {
        if(start==end)return;
        int idx=static_cast<int>(*start);
        start++;
        switch (idx)
        {
        case 0:
            stringify(os,decode(os,start,static_cast<std::tuple_element_t<0,SupportedTypes>*>(nullptr)),end);
        }
    }

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
            return write_state[sz].fetch_add(1,std::memory_order_acquire)+1==sz;
        }

        bool try_pop(LogLine& lg,const unsigned int read_idx)
        {
            unsigned int state=write_state[read_idx].load(std::memory_order_acquire);
            if(!state)return false;
            lg=std::move(buffer[read_idx].lg);
            return true;
        }

        Buffer(const Buffer&)=delete;
        Buffer& operator=(const Buffer&)=delete;
    private:
        Item* buffer;
        std::atomic<unsigned int> write_state[sz+1];
    };

    class QueueBuffer
    {
    public:
        QueueBuffer():cur_read_buffer(nullptr),flag(ATOMIC_FLAG_INIT),write_index(0)
        {
            setup_new_buffer();
        }

        void push(LogLine&& lg)
        {
            unsigned int next_write=write_index.fetch_add(1,std::memory_order_relaxed);
            if(next_write<Buffer::sz)
            {
                if(cur_write_buffer.load(std::memory_order_acquire)->push(std::move(lg)))
                    setup_new_buffer();
            }
            else
            {
                while(write_index.load(std::memory_order_acquire)>=Buffer::sz);
                push(std::move(lg));
            }
        }

        void setup_new_buffer()
        {
            std::unique_ptr<Buffer> next_buffer(new Buffer());
            cur_write_buffer.store(next_buffer.get(),std::memory_order_release);
            SpinLock sl(flag);
            buffers.push(std::move(next_buffer));
            write_index.store(0,std::memory_order_relaxed);
        }
    private:
        //多个线程的消费者共同访问，需要使用原子变量或者加锁
        std::queue<std::unique_ptr<Buffer>> buffers;
        std::atomic<Buffer*> cur_write_buffer;
        std::atomic<unsigned int> write_index;
        std::atomic_flag flag;
        //主线程读取的变量，不存在竞争
        Buffer* cur_read_buffer;
    };


    class write_to_file
    {
    public:
        write_to_file(const std::string& dir,const std::string& file,uint32_t roll_size):
        write_to(dir+file),roll_size_bytes(roll_size*1024*1024)
        {

        }

        void write(LogLine& lg)
        {

        }
    private:
        const std::string write_to;
        const uint32_t roll_size_bytes;
        uint32_t file_number=0;
    };

    class LittleLogger
    {
    public:
        LittleLogger(const std::string& dir,const std::string& file,uint32_t roll_size):
        state(State::INTI),log_buffer(new QueueBuffer()),writer(dir,file,roll_size),
        read_thread(&LittleLogger::work,this)
        {
            state.store(State::READY,std::memory_order_release);
        }

        ~LittleLogger()
        {
            state.store(State::SHOUTDOWN);
            read_thread.join();
        }

        void add(LogLine&& lg)
        {
            log_buffer->push(std::move(lg));
        }

        void work()
        {

        }
    private:
        enum class State{
            INTI,READY,SHOUTDOWN
        };
        std::atomic<State> state;
        std::unique_ptr<QueueBuffer> log_buffer;
        write_to_file writer;
        std::thread read_thread;
    };

    std::unique_ptr<LittleLogger> littlelog;
    std::atomic<LittleLogger*> atomic_littlelog;

    /**
     * @brief 不同的线程通过此重载运算符向日志主线程中放入新的日志信息
     * 
     * @param lg 
     * @return true 
     * @return false 
     */
    bool Log::operator+=(LogLine& lg)
    {
        atomic_littlelog.load(std::memory_order_acquire)->add(std::move(lg));
        return true;
    }

    void init(const std::string& directory,const std::string& file,uint32_t roll_size)
    {
        littlelog.reset(new LittleLogger(directory,file,roll_size));
        atomic_littlelog.store(littlelog.get(),std::memory_order_seq_cst);
    }
}




