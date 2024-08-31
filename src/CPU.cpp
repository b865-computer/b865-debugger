#include "CPU.h"

uint32_t AddrModeTable[16][16];

void CPU::init()
{
    AC = A = B = IR0 = IR1 = SP = 0;
    ADDRESS.H = 0x80;
    ADDRESS.L = 0x00;
    PC.addr = 0x8000;
    isComputing = false;
    isFeching = false;
    isFechingData = false;
}

void CPU::cycle()
{
    if (isFeching)
    {
        if (Istate == 0)
        {
            IR0 = mem.get();
            PC.addr++;
            mem.setAddress(PC.addr);
        }
        else
        {
            IR1 = mem.get();
            PC.addr++;
            mem.setAddress(PC.addr);
            isFeching = false;
            isFechingData = true;
            fechCycle = 0;
        }
    }
    else if (isFechingData)
    {
        signals.val = AddrModeTable[addrMode][fechCycle];
        if(!isFechingData)
        {
            isComputing = true;
        }
        fechCycle++;
    }
    if (isComputing)
    {
    }
    Istate++;
}
