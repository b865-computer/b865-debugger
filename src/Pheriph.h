#pragma once

#ifndef _PERIPH_H_
#define _PERIPH_H_
#include "Common.h"

class Pheriph
{
public:
    Pheriph();
    Pheriph(uint16_t len);
    Pheriph(uint16_t len, std::string name);
    Pheriph(uint16_t len, std::string name, std::vector<std::string> regNames);
    ~Pheriph();
    void stop();

private:
    void initRegs(int);

public:
    bool running = false;
    uint8_t *regs;
    std::string m_name;
    std::vector<std::string> m_regNames;

private:
};

#endif