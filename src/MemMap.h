#pragma once

#ifndef _MEMMAP_H_
#define _MEMMAP_H_

#include "Computer.h"
#include "Pheriph.h"

class MEMMAP
{
public:
    enum REG_TYPE{
        pheriph,
        ram,
        rom
    };
    inline MEMMAP(REG_TYPE _type, Pheriph* _p, uint16_t _start, uint32_t _len)
    :p(*_p)
    {

    }
    REG_TYPE type = ram;
    Pheriph &p;
    uint16_t start = 0;
    uint16_t len = 0;
};
#endif