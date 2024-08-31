#pragma once

#ifndef _MEM_H_
#define _MEM_H_

#include "Computer.h"
#define MEM_REG_COUNT 1
#include "MemMap.h"

class MEM
{
public:
    byte get();
    void setAddress(uint16_t new_addr);
private:
    byte data_array[0x10000];
    uint16_t address;
    static MEMMAP MemMap[MEM_REG_COUNT];
};

#endif // _MEM_H_