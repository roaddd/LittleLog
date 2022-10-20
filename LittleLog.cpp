#include "LittleLog.hpp"
#include <chrono>
#include <thread>
#include <cstring>
#include <string>
#include <ostream>

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
    static const constexpr std::size_t value=1+TupleIndex<T,Types...>; 
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


