#pragma once
#ifndef _EMULATOR_H_
#define _EMULATOR_H_

class Emulator;

#include "Common.h"
#include "gui.h"
#include "CPU.h"
#include "Clock.h"
#include "Debugger.h"
#include <CdbgExpr.h>

void cycle();

class Emulator : public CdbgExpr::DbgData
{
public:
    Emulator();
    int init();
    int load(std::string filename, std::string path = "");
    int load(std::vector<uint8_t> &programData);
    int main();
    std::chrono::nanoseconds getRunTime_ns();

public:
    CdbgExpr::SymbolDescriptor sym;
    
    CdbgExpr::SymbolDescriptor& getSymbol(const std::string &name) override;
    uint8_t getByte(uint64_t address) override;
    void setByte(uint64_t address, uint8_t value) override;
    constexpr uint8_t CTypeSize(CdbgExpr::CType type) override;

private:
    void start();
    void stop();
    bool isRunning();

private:
    Clock m_clock;
    CPU &m_cpu;
    GUI m_gui;
    FQ m_fq;
    DebuggerDataHelper m_debuggerData;
};

#endif // _COMPUTER_H_