#ifndef LITTLELOG_HPP
#define LITTLELOG_HPP

#include <stdint.h>
#include <string>
#include <memory>

enum class LogLevel:uint8_t
{
    INFO,WARN,DEBUG
};

class LogLine
{
public:
    LogLine(LogLevel level,const char* dir,const char* file,const char* function,uint32_t line);
    ~LogLine();

    LogLine(LogLine &&)=default;
    LogLine& operator=(LogLine&&)=default;

    // LogLine& operator<<(char arg);
    // LogLine& operator<<(int32_t arg);
    // LogLine& operator<<(uint32_t arg);
    // LogLine& operator<<(int64_t arg);
    // LogLine& operator<<(int64_t arg);
    // LogLine& operator<<(int64_t arg);
    // LogLine& operator<<(const std::string& arg);
    // LogLine& operator<<(const char* arg);
    // LogLine& operator<<(char* arg);
    // LogLine& operator<<(const char (&arg)[]);

    template<typename Arg>
    void operator<<(Arg arg)
    {
        encode(arg);
        return *this;
    }

    template<>
    void operator<< <const char* const >(const char* const& arg)
    {
        
    }

    // template<size_t N>
    // LogLine& operator<<(const char(&arg)[N])
    // {
    //     encode(string_literal_t(arg));
    //     return *this;
    // }

    
    

    struct string_literal_t
    {
        explicit string_literal_t(const char* s_):s(s_){}
        const char* s;
    };

private:
    char* get_index();



    size_t bytes_used,buffer_size;
    std::unique_ptr<char[]> heaq_buffer;
    char stack_buffer[256-2*sizeof(size_t)-sizeof(decltype(heaq_buffer))];
};

struct Log
{
    bool operator+=(LogLine &);
};

struct QueueLogger
{

};


void set_level(LogLevel lg);
bool level_isvalid(LogLevel lg);

void init(QueueLogger ql,std::string log_dir,std::string log_file,uint32_t roll_size);

#define LOG(level) Log()+=LogLine()
#define LOG_INFO level_isvalid(LogLevel::INFO) && LOG(LogLevel::INFO)
#define LOG_WARN level_isvalid(LogLevel::WARN) && LOG(LogLevel::WARN)
#define LOG_WARN level_isvalid(LogLevel::DEBUG) && LOG(LogLevel::DEBUG)

#endif