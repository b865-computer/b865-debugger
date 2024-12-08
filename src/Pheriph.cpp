#include "Pheriph.h"
#include <thread>

Pheriph::Pheriph(uint16_t len, void (*thread_func)(uint8_t*, bool*))
{
    if (len)
    {
        regs = (uint8_t *)calloc(1, len);
        if (!regs)
        {
            fprintf(stderr, "Failed to allocate: %i uint8_ts", len);
            exit(1);
        }
    }
    if(thread_func)
    {
        thread = std::thread(thread_func, regs, &end);
        running = true;
    }
}

Pheriph::~Pheriph()
{
    stop();
    if(regs)
    {
        delete regs;
        regs = nullptr;
    }
    return;
}

void Pheriph::stop()
{
    if(!running)
    {
        return;
    }
    end = true;
    if(thread.joinable())
    {
        thread.join();
    }
    running = false;
}

Pheriph noPheriph(0, nullptr);