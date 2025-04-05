#pragma once
#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include "Common.h"
#include "gui.h"
#include "CPU.h"
#include "Clock.h"
#include "Debugger.h"

void cycle();

class Emulator
{
public:
    Emulator();
    int init();
    int load(std::string filename, std::string path = "");
    int load(std::vector<uint8_t> &programData);
    int main();
    std::chrono::nanoseconds getRunTime_ns();

private:
    void start();
    void stop();
    bool isRunning();

private:
    Clock m_clock;
    CPU &m_cpu;
    GUI m_gui;
    FQ m_fq;
    DebuggerData m_debuggerData;
};

#endif // _COMPUTER_H_