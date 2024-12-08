#pragma once

#ifndef _PERIPH_H_
#define _PERIPH_H_
#include "Common.h"
#include <thread>

class Pheriph
{
public:
    Pheriph(uint16_t len, void (*thread_func)(uint8_t*, bool*));
    ~Pheriph();
    uint8_t* regs;
    bool end = false;
    std::thread thread;
};

void IO_func(uint8_t*, bool*); // Pheriph_IO.cpp

extern Pheriph noPheriph;

#endif