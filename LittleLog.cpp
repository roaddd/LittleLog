#include "LittleLog.hpp"
#include <chrono>
#include <thread>
#include <cstring>
#include <string>
#include <ostream>
#include <queue>
#include <atomic>
#include <fstream>
#include <iostream>

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
        std::cout<<"encode"<<std::endl;
        encode<string_literal_t>(arg,TupleIndex<string_literal_t,SupportedTypes>::value);
    }

    LogLine::LogLine(LogLevel level,const char* file,const char* function,uint32_t line)
    :bytes_used(0),buffer_size(sizeof(stack_buffer))
    {
        encode<uint64_t> (timestamp());
        encode<std::thread::id> (this_thread_id());
        encode<string_literal_t> (string_literal_t(file));
        encode<string_literal_t> (string_literal_t(function));
        encode<uint32_t> (line);
        encode<LogLevel>(level);
    }

    LogLine::~LogLine()=default;

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

    /**
     * @brief 格式化输出函数
     * 
     * @param os :输出文件流
     */
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
        std::cout<<"stringfy"<<std::endl;
    }

    /**
     * @brief 针对不同数据类型的输出函数，供stringify格式化函数使用
     * 
     * @tparam Arg 
     * @param os 
     * @param b 
     * @param dummy 
     * @return char* 
     */
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

    /**
     * @brief 此函数递归地调用，格式化不同类型的数据
     * 
     * @param os 
     * @param start 
     * @param end 
     */
    void LogLine::stringify(std::ostream& os,char* start,const char* end)
    {
        if(start==end)return;
        int idx=static_cast<int>(*start);
        start++;
        switch (idx)
        {
            case 0:
                stringify(os,decode(os,start,static_cast<std::tuple_element_t<0,SupportedTypes>*>(nullptr)),end);
                return;
            case 1:
                stringify(os,decode(os,start,static_cast<std::tuple_element_t<1,SupportedTypes>*>(nullptr)),end);
                return;
            case 2:
                stringify(os,decode(os,start,static_cast<std::tuple_element_t<2,SupportedTypes>*>(nullptr)),end);
                return;
            case 3:
                stringify(os,decode(os,start,static_cast<std::tuple_element_t<3,SupportedTypes>*>(nullptr)),end);
                return;
            case 4:
                stringify(os,decode(os,start,static_cast<std::tuple_element_t<4,SupportedTypes>*>(nullptr)),end);
                return;
            case 5:
                stringify(os,decode(os,start,static_cast<std::tuple_element_t<5,SupportedTypes>*>(nullptr)),end);
                return;
            case 6:
                stringify(os,decode(os,start,static_cast<std::tuple_element_t<6,SupportedTypes>*>(nullptr)),end);
                return;
            case 7:
                stringify(os,decode(os,start,static_cast<std::tuple_element_t<7,SupportedTypes>*>(nullptr)),end);
                return;
        }
    }

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
            std::cout<<"push"<<write_state[new_idx].load()<<std::endl;
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

    /**
     * @brief 日志信息缓冲区的环形队列，用来存放Buffer
     * 
     */
    class QueueBuffer
    {
    public:
        QueueBuffer():cur_read_buffer(nullptr),flag(ATOMIC_FLAG_INIT),write_index(0),read_index(1)
        {
            setup_new_buffer();
        }

        void push(LogLine&& lg)
        {
            unsigned int next_write=write_index.fetch_add(1,std::memory_order_relaxed);
            if(next_write<Buffer::sz)
            {
                std::cout<<"write_index"<<write_index<<std::endl;
                if(cur_write_buffer.load(std::memory_order_acquire)->push(std::move(lg),write_index))
                    setup_new_buffer();
            }
            else
            {
                while(write_index.load(std::memory_order_acquire)>=Buffer::sz);
                push(std::move(lg));
            }
        }

        void get_next_read_buffer()
        {
            SpinLock sp(flag);
            cur_read_buffer=buffers.size()?buffers.front().get():nullptr;
        }

        bool try_pop(LogLine& lg)
        {
            if(cur_read_buffer==nullptr)
                get_next_read_buffer();
            if(cur_read_buffer==nullptr)
                return false;
            Buffer* bf=cur_read_buffer;
            std::cout<<(bf==nullptr)<<std::endl;
            if(bool succedd=bf->try_pop(lg,read_index))
            {
                std::cout<<succedd<<std::endl;
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

        void setup_new_buffer()
        {
            std::unique_ptr<Buffer> next_buffer(new Buffer());
            cur_write_buffer.store(next_buffer.get(),std::memory_order_release);
            SpinLock sl(flag);
            buffers.push(std::move(next_buffer));
            write_index.store(0,std::memory_order_relaxed);
        }
    private:
        //保证数据同步
        //多个线程的消费者共同访问，需要使用原子变量或者加锁
        std::queue<std::unique_ptr<Buffer>> buffers;
        std::atomic<Buffer*> cur_write_buffer;
        std::atomic<unsigned int> write_index;
        std::atomic_flag flag;
        //主线程读取的变量，不存在竞争
        Buffer* cur_read_buffer;
        unsigned int read_index;
    };

    /**
     * @brief 向文件中写日志信息
     * 
     */
    class write_to_file
    {
    public:
        write_to_file(const std::string& dir,const std::string& file,uint32_t roll_size):
        write_to(dir+file),roll_size_bytes(roll_size*1024*1024)
        {
            roll_file();
        }

        void write(LogLine& lg)
        {
            std::cout<<"write"<<std::endl;
            auto pos=os->tellp();
            std::cout<<pos<<std::endl;
            lg.stringify(*os);
            bytes_writed+=os->tellp()-pos;
            if(bytes_writed>=roll_size_bytes)
                roll_file();
        }

        void roll_file()
        {
            if(os)
            {
                os->flush();
                os->close();
            }
            bytes_writed=0;
            os.reset(new std::ofstream());
            std::string file_name=write_to;
            file_name.append(".");
            file_name.append(std::to_string(file_number));
            file_number++;
            file_name.append(".txt");
            os->open(file_name,std::ofstream::out|std::ofstream::trunc);
            std::cout<<file_name<<std::endl;
        }
    private:
        std::unique_ptr<std::ofstream> os;
        const std::string write_to;
        const uint32_t roll_size_bytes;
        uint32_t file_number=0;
        uint32_t bytes_writed=0;
    };

    /**
     * @brief 实现了日志系统的后台线程，该线程不断地检查缓冲区队列中是否存在未写入文件的日志；
     *      如有，取出写入文件，若没有，则sleep
     * 
     */
    class LittleLogger
    {
    public:
        LittleLogger(const std::string& dir,const std::string& file,uint32_t roll_size):
        state(State::INTI),log_buffer(new QueueBuffer()),writer(dir,file,roll_size),
        read_thread(&LittleLogger::work,this)
        {
            state.store(State::READY,std::memory_order_release);
            std::cout<<static_cast<uint32_t>(state.load(std::memory_order_acquire))<<std::endl;
        }

        ~LittleLogger()
        {
            state.store(State::SHOUTDOWN);
            read_thread.join();
        }

        void add(LogLine&& lg)
        {
            std::cout<<"add"<<std::endl;
            log_buffer->push(std::move(lg));
        }

        void work()
        {
            std::cout<<"work"<<std::endl;
            while(state.load(std::memory_order_acquire)==State::INTI)
                std::this_thread::sleep_for(std::chrono::microseconds(50));
            LogLine curLog(LogLevel::INFO,nullptr,nullptr,0);
            //std::cout<<static_cast<uint32_t>(state.load())<<std::endl;
            while(state.load()==State::READY)
            {
                if(log_buffer.get()->try_pop(curLog))
                    writer.write(curLog);
                else
                    std::this_thread::sleep_for(std::chrono::microseconds(50));
            }
            while(log_buffer->try_pop(curLog))
                writer.write(curLog);
            std::cout<<"---work---"<<std::endl;
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
    bool Log::operator==(LogLine& lg)
    {
        std::cout<<"*************"<<std::endl;
        atomic_littlelog.load(std::memory_order_acquire)->add(std::move(lg));
        return true;
    }

    std::atomic<unsigned int> loglevel(0);

    void set_level(LogLevel lg)
    {
        loglevel.store(static_cast<unsigned int>(lg),std::memory_order_release);
    }

    bool level_isvalid(LogLevel lv)
    {
        return static_cast<unsigned int>(lv)>=loglevel.load(std::memory_order_relaxed);
    }

    void init(const std::string& directory,const std::string& file,uint32_t roll_size)
    {
        littlelog.reset(new LittleLogger(directory,file,roll_size));
        atomic_littlelog.store(littlelog.get(),std::memory_order_seq_cst);
    }
}




