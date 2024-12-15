#include "MEM.h"

MEMORY::MEMORY(MEMMAP *_MemMap, int count)
    : MemMap(_MemMap)
{
    m_region_count = count;
}

uint8_t MEMORY::get(uint16_t address)
{
    for (int i = 0; i < MEM_REGION_COUNT; i++)
    {
        if (MemMap[i].start <= address && (MemMap[i].len + MemMap[i].start) > address)
        {
            if (MemMap[i].type == MEMMAP::REGION_TYPE::pheriph)
            {
                return MemMap[i].p.regs[address - MemMap[i].start];
            }
            else
            {
                return data_array[address];
            }
        }
    }
    return 0;
}

void MEMORY::set(uint16_t address, uint8_t data)
{
    for (int i = 0; i < MEM_REGION_COUNT; i++)
    {
        if (MemMap[i].start <= address && (MemMap[i].len + MemMap[i].start) > address)
        {
            if (MemMap[i].type == MEMMAP::REGION_TYPE::pheriph)
            {
                MemMap[i].p.regs[address - MemMap[i].start] = data;
            }
            else if (MemMap[i].type == MEMMAP::REGION_TYPE::rom)
            {
                return;
            }
            else if (MemMap[i].type == MEMMAP::REGION_TYPE::ram)
            {
                data_array[address] = data;
                return;
            }
        }
    }
}
void MEMORY::cpy(uint16_t firstAdr, uint8_t *_data, uint16_t len)
{
    int i = firstAdr;
    while (i < (len + firstAdr))
    {
        for (int r = 0; r < MEM_REGION_COUNT; r++)
        {
            if (MemMap[r].start <= i && (MemMap[r].len + MemMap[r].start) > i)
            {
                int regionLen = MemMap[r].len;
                if (regionLen + i - firstAdr > len)
                {
                    regionLen = len - (i - firstAdr);
                }
                cpyRegion(firstAdr, _data, regionLen, r);
                i += regionLen;
            }
        }
    }
}

void MEMORY::stopPheripherials()
{
    for (int i = 0; i < MEM_REGION_COUNT; i++)
    {
        if (MemMap[i].type == MEMMAP::REGION_TYPE::pheriph)
        {
            MemMap[i].p.stop();
        }
    }
}

Pheriph **MEMORY::getPheripherials(int *_count)
{
    int count = 0;

    for (int i = 0; i < MEM_REGION_COUNT; i++)
    {
        if (MemMap[i].type == MEMMAP::REGION_TYPE::pheriph)
        {
            count++;
        }
    }

    (*_count) = count;
    Pheriph **pheriph_list = new Pheriph *[count];

    count = 0;
    for (int i = 0; i < MEM_REGION_COUNT; i++)
    {
        if (MemMap[i].type == MEMMAP::REGION_TYPE::pheriph)
        {
            pheriph_list[count] = &MemMap[i].p;
            count++;
        }
    }
    return pheriph_list;
}

/**
 * Warning: This function does not check if the data fist into the memory region.
 */
void MEMORY::cpyRegion(uint16_t firstAdr, uint8_t *_data, uint16_t len, int regID)
{
    int i = regID;
    if (MemMap[i].start <= firstAdr && (MemMap[i].len + MemMap[i].start) > firstAdr)
    {
        if (MemMap[i].type == MEMMAP::REGION_TYPE::pheriph)
        {
            for (int j = 0; j < len; j++)
            {
                MemMap[i].p.regs[firstAdr + j - MemMap[i].start] = _data[j];
            }
        }
        else
        {
            for (int j = 0; j < len; j++)
            {
                data_array[firstAdr + j] = _data[j];
            }
        }
    }
    return;
}