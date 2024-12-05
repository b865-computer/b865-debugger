#pragma once

#ifndef _MEMMAP_H_
#define _MEMMAP_H_

#include "Computer.h"
#include "Pheriph.h"

class MEMMAP
{
public:
    enum REGION_TYPE{
        pheriph,
        ram,
        rom
    };
    inline MEMMAP(REGION_TYPE _type, uint16_t _start, uint32_t _len)
    {

    }
    inline MEMMAP(Pheriph* _p, uint16_t _start, uint32_t _len)
    :p(*_p)
    {
        type = REGION_TYPE::pheriph;
    }
    REGION_TYPE type = ram;
    Pheriph &p = noPheriph;
    uint16_t start = 0;
    uint16_t len = 0;
};
#endif