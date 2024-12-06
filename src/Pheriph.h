#pragma once

#ifndef _PERIPH_H_
#define _PERIPH_H_
#include "Computer.h"
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

extern Pheriph noPheriph;

#endif