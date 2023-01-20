#include "Write_to_file.hpp"

namespace littlelog
{
    write_to_file::write_to_file(const std::string& dir,const std::string& file,uint32_t roll_size):
    write_to(dir+file),roll_size_bytes(roll_size*1024*1024)
    {
        roll_file();
    }   

    void write_to_file::write(LogLine& lg)
    {
        //std::cout<<"write"<<std::endl;
        auto pos=os->tellp();
        //std::cout<<pos<<std::endl;
        lg.stringify(*os);
        bytes_writed+=os->tellp()-pos;
        if(bytes_writed>=roll_size_bytes)
            roll_file();
    }

    void write_to_file::roll_file()
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
        //std::cout<<file_name<<std::endl;
    }

}