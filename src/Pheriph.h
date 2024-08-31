#pragma once

#ifndef _PERIPH_H_
#define _PERIPH_H_
#include "Computer.h"

struct Pheriph
{
    Pheriph(uint8_t len);
    uint8_t* regs;
};

#endif