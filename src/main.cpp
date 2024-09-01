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
        sleep = ns / 2;
        return;
    }
    uint64_t sleep = 10;
    uint64_t ns = 10;
    uint64_t HZ = 10;
};

FQ frequency(1000000);
CPU cpu;

int main(int argc, char *argv[])
{
    cpu.init();
    auto start = std::chrono::high_resolution_clock::now();

    while (1)
    {
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        uint64_t nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
        if (nanoseconds > frequency.ns)
        {
            start = std::chrono::high_resolution_clock::now();
            cpu.cycle();
<<<<<<< Updated upstream
=======
            counter++;
            if(counter >= frequency.HZ / 100)
            {
                fprintf(stderr,"\n");
                return 0;
            }
>>>>>>> Stashed changes
        }
        std::this_thread::sleep_for(std::chrono::nanoseconds(frequency.sleep));
    }
    return 0;
}