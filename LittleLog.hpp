#ifndef LITTLELOG_HPP
#define LITTLELOG_HPP

#include <stdint.h>
#include <string.h>
#include <memory>

namespace littlelog
{

    enum class LogLevel:uint8_t
    {
        INFO,WARN,DEBUG
    };

    /**
     * @brief 日志条目类
     * 
     */
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
        LogLine& operator<<(Arg arg)
        {
            encode(arg);
            return *this;
        }

        template<size_t N>
        LogLine& operator<<(const char(&arg)[N])//" "
        {
            encode(string_literal_t(arg));
            return *this;
        }
        

        struct string_literal_t
        {
            explicit string_literal_t(const char* s_):s(s_){}
            const char* s;
        };

        void stringify(std::ostream& os);
    private:
        char* get_index();

        template<typename Arg>
        void encode(Arg arg)
        {
            *reinterpret_cast<Arg*>(get_index())=arg;
            bytes_used+=sizeof(arg);
        }

        template<typename Arg>
        void encode(Arg arg,uint8_t type_id)
        {
            resize_buffer(sizeof(arg)+sizeof(type_id));
            encode<uint8_t>(type_id);
            encode<Arg>(arg);
        }

        void encode(char* arg);
        void encode(const char* arg);
        void encode(string_literal_t arg);
        void encode_c_string(const char* arg,size_t length);
        void resize_buffer(size_t sz);
        
        void stringify(std::ostream& os,char* start,const char* end);

        size_t bytes_used,buffer_size;
        std::unique_ptr<char[]> heap_buffer;
        char stack_buffer[256-2*sizeof(size_t)-sizeof(decltype(heap_buffer))];
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
}


#define LOG(level) (Log()+=LogLine())
#define LOG_INFO level_isvalid(LogLevel::INFO) && LOG(LogLevel::INFO)
#define LOG_WARN level_isvalid(LogLevel::WARN) && LOG(LogLevel::WARN)
#define LOG_WARN level_isvalid(LogLevel::DEBUG) && LOG(LogLevel::DEBUG)

#endif