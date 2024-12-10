#pragma once

#ifndef _MEM_H_
#define _MEM_H_

#include "Common.h"
#define MEM_REGION_COUNT m_region_count
#include "MemMap.h"

class MEMORY
{
public:
    MEMORY(MEMMAP*, int count);
    uint8_t get(uint16_t address);
    void set(uint16_t address, uint8_t data);
    void cpy(uint16_t firstAdr, uint8_t *_data, uint16_t len);
    void stopPheripherials();
    Pheriph** getPheripherials(int* count);

private:
    void cpyRegion(uint16_t firstAdr, uint8_t *_data, uint16_t len, int regID);

private:
    uint8_t data_array[0x10000];
    int m_region_count = 0;
    MEMMAP* MemMap;
};

#endif // _MEM_H_