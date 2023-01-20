#ifndef __WRITE_TO_FILE__
#define __WRITE_TO_FILE__

#include <string>
#include "LittleLog.hpp"
#include <fstream>

namespace littlelog
{
    /**
 * @brief 向文件中写日志信息
 * 
 */
class write_to_file
{
public:
    write_to_file(const std::string& dir,const std::string& file,uint32_t roll_size);

    void write(LogLine& lg);
    
    void roll_file();

private:
    std::unique_ptr<std::ofstream> os;
    const std::string write_to;
    const uint32_t roll_size_bytes;
    uint32_t file_number=0;
    uint32_t bytes_writed=0;
};

}

#endif