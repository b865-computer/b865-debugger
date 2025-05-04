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
    
    CdbgExpr::SymbolDescriptor getSymbol(const std::string &name) override;
    uint8_t getByte(uint64_t address) override;
    void setByte(uint64_t address, uint8_t value) override;
    uint8_t CTypeSize(CdbgExpr::CType type) override;

public:
    DebuggerDataHelper m_debuggerData;

private:
    SymbolRecord getSymbolRecord(const std::string &name);
    LinkerRecord getSymbolAddr(const std::string &name);
    void start();
    void stop();
    bool isRunning();

private:
    Clock m_clock;
    CPU &m_cpu;
    GUI m_gui;
    FQ m_fq;
};

#endif // _COMPUTER_H_