#include "Pheriph.h"

void IO_func(uint8_t* regs, bool *end)
{
    if(!regs)
    {
        _sleep(100);
        fprintf(stderr, "Error: IO_func called with NULL pointer.\n");
        return;
    }
    uint8_t old_regs[0x100];
    for(int i = 0; i < 0x100; i++)
    {
        old_regs[i] = regs[i];
    }
    while(!(*end))
    {
        if(old_regs[1] != regs[1])
        {
            fprintf(stdout, "LED: %s\r", (regs[1] & regs[0] & 0x01) ? "*" : " ");
            old_regs[1] = regs[1];
        }
    }
    fprintf(stdout, "LED: %s\n", regs[1] ? "*" : " ");
    fprintf(stderr, "IO_func terminated.\n");
    return;
}