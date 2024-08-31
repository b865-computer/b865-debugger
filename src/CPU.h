#pragma once

#ifndef _CPU_H_
#define _CPU_H_

#include <iostream>
#include "Computer.h"
#include "MEM.h"

union uint16_ADDR
{
    uint16_t addr;
    struct
    {
        byte L;
        byte H;
    };
};

struct CPU_State
{
    byte AC, A, B, IR0, IR1, SP;
    uint16_ADDR PC, ADDRESS;
    byte Istate;
};

union Signals
{
    struct
    {
        uint8_t HLT         : 1;
    };
    
    uint32_t val;
};

class CPU: public CPU_State
{
public:
    void init();
    void cycle();

private:
    void doDataFechCycle();

private:
    MEM mem;
    Signals signals;
    byte Istate = 0;
    bool isFeching = true;
    bool isFechingData = false;
    bool isComputing = false;
    int fechCycle = 0;
    byte addrMode = 0, addrMode2 = 0, RI = 0, RO= 0;
};

#endif // _CPU_H_