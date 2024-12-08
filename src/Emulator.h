#pragma once
#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include "Common.h"
#include "gui.h"
#include "CPU.h"
#include "Clock.h"

void cycle();

class Emulator
{
public:
    Emulator();
    void init();
    int load(std::string filename);
    int load(std::vector<uint8_t>& programData);
    void start();
    void stop();
    std::chrono::nanoseconds getRunTime_ns();
    bool isRunning();
    int exitCode();

private:

private:
    Clock m_clock;
    CPU& m_cpu;
    GUI m_gui;
    FQ m_fq;
    int m_exitCode = 0;
};

#endif // _COMPUTER_H_