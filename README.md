# LittleLog
## LittleLog介绍
该项目仿照NanoLog实现了一个简易的日志系统，测试方法：
新建Build文件夹，然后执行 **cmake ../** 命令生成makefile文件，执行 **make** 命令，build/lib文件夹中生成liblittlelog.a静态库，build/bin文件夹中生成test可执行文件，执行test可得到该日志系统的测试结果。

添加了编译选项(cmake -DXXX ../)：
* -DTERMINAL_DISPLAY=ON 向文件写的同时向终端输出日志信息，默认为不向终端输出
## LittleLog性能测试
开启五个写线程，每个写线程向日志系统写入100条日志：
```
void work()
{
    const int cnt=100;
    const char* c="benchmark";
    uint64_t start=std::chrono::steady_clock::now().time_since_epoch() / std::chrono::microseconds(1);
    for (int i = 0; i < cnt; ++i)
	    LOG_INFO <<i;
	std::string s = { 'a' };
    LOG_INFO<<s;
	LOG_INFO <<"abckso"<<9<<'k'<<5679812;
    uint64_t end = std::chrono::steady_clock::now().time_since_epoch() / std::chrono::microseconds(1);
    long int avg_latency = (end - start) * 1000 / cnt;
    printf("\tAverage LittleLog Latency = %ld nanoseconds\n", avg_latency);
}
```
测试结果：
```
        Average LittleLog Latency = 520 nanoseconds
        Average LittleLog Latency = 520 nanoseconds
        Average LittleLog Latency = 630 nanoseconds
        Average LittleLog Latency = 630 nanoseconds
        Average LittleLog Latency = 600 nanoseconds
```

