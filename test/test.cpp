#include <iostream>
#include "LittleLog.hpp"
#include <vector>
#include <chrono>
#include <thread>


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
    long int avg_latency = (end - start) * 100 / cnt;
    printf("\tAverage LittleLog Latency = %ld nanoseconds\n", avg_latency);
}

template<typename Fun>
void benchmark(Fun&& f,int count)
{
    std::vector<std::thread> threads;
    for(int i=0;i<count;i++)threads.push_back(std::thread(f));
    for(int i=0;i<count;i++)threads[i].join();
}

int main()
{
    littlelog::init("/tmp/","log",1);
    benchmark(work,3);
    return 0;
}
