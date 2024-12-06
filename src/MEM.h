#pragma once

#ifndef _MEM_H_
#define _MEM_H_

#include "Computer.h"
#define MEM_REGION_COUNT 4
#include "MemMap.h"

class MEMORY
{
public:
    byte get(uint16_t address);
    void set(uint16_t address, byte data);
    void cpy(uint16_t firstAdr, uint8_t *_data, uint16_t len);

private:
    void cpyRegion(uint16_t firstAdr, uint8_t *_data, uint16_t len, int regID);

private:
    byte data_array[0x10000];
    static MEMMAP MemMap[MEM_REGION_COUNT];
};

#endif // _MEM_H_