#pragma once

#ifndef _PERIPH_H_
#define _PERIPH_H_
#include "Computer.h"

class Pheriph
{
public:
    Pheriph(uint8_t len);
    uint8_t* regs;
};

extern Pheriph noPheriph;

#endif