#include "MEM.h"

byte MEM::get()
{
    return data_array[address];
}

void MEM::setAddress(uint16_t new_addr)
{
    address = new_addr;
}

Pheriph noPheriph(0);

MEMMAP MEM::MemMap[MEM_REG_COUNT] = {MEMMAP(MEMMAP::REG_TYPE::ram,&noPheriph,0,0x10000)};