#pragma once

#ifndef _MEMMAP_H_
#define _MEMMAP_H_

#include "Common.h"
#include "Pheriph.h"

static Pheriph noPheriph;

class MEMMAP
{
public:
    enum REGION_TYPE
    {
        pheriph,
        ram,
        rom
    };
    inline MEMMAP(REGION_TYPE _type, uint16_t _start, uint32_t _len)
    {
        start = _start;
        len = _len;
        type = type;
    }
    inline MEMMAP(Pheriph *_p, uint16_t _start, uint32_t _len)
        : p(*_p)
    {
        type = REGION_TYPE::pheriph;
        start = _start;
        len = _len;
    }
    REGION_TYPE type = ram;
    Pheriph &p = noPheriph;
    uint16_t start = 0;
    uint16_t len = 0;
};
#endif