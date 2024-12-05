#include <iostream>
#include <cstdio>
#include <chrono>
#include <thread>
#include "CPU.h"

class FQ
{
public:
    inline FQ(uint64_t _HZ)
    {
        HZ = _HZ;
        ns = 1000000000 / HZ;
        sleep = ns / 5;
        return;
    }
    uint64_t sleep = 10;
    uint64_t ns = 10;
    uint64_t HZ = 10;
};

FQ frequency(1000000);
CPU cpu;
unsigned long long counter = 0; // not it's not gonna overflow for 584 years. (at 1 GHz)

int main(int argc, char *argv[])
{
    cpu.init();
    cpu.startExec();

    fprintf(stdout, "Start...\n");

    auto start = std::chrono::high_resolution_clock::now();
    auto now = start;
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    
    while (1)
    {
        now = std::chrono::high_resolution_clock::now();
        elapsed = now - (start + std::chrono::nanoseconds(counter * frequency.ns));
        uint64_t nanoseconds = elapsed.count();
        if (nanoseconds > frequency.ns)
        {
            cpu.cycle();
            counter++;
            if(counter >= frequency.HZ)
            {
                auto end = std::chrono::high_resolution_clock::now();
                fprintf(stdout,"Finished, time: %lldns\n", (end - start).count());
                return 0;
            }
        }
        std::this_thread::sleep_for(std::chrono::nanoseconds(frequency.sleep));
    }
    return 0;
}