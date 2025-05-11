#pragma once
#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include "Common.h"
#include "CPU.h"
#include "Clock.h"
#include "Debugger.h"
#include <CdbgExpr.h>
#include <functional>

void cycle();

class Emulator : public CdbgExpr::DbgData
{
public:
    Emulator(std::function<void(const std::string&)> error_callback);
    ~Emulator();
    int init();
    int load(std::string filename);
    int load(std::vector<uint8_t> &programData);
    int main();
    void terminate();
    std::chrono::nanoseconds getRunTime_ns();
    void setInsLevel(bool);
    
    void start();
    void stop();
    bool pausedAtBreakpoint();
    void pause();
    bool clockRunning();
    void continue_exec();
    
    CdbgExpr::SymbolDescriptor getSymbol(const std::string &name) override;
    uint8_t getByte(uint64_t address) override;
    void setByte(uint64_t address, uint8_t value) override;
    uint8_t CTypeSize(CdbgExpr::CType type) override;
    uint64_t getStackPointer() override;
    uint8_t getRegContent(uint8_t regNum) override;
    void setRegContent(uint8_t regNum, uint8_t val) override;

public:
    DebuggerDataHelper m_debuggerData;
    Clock m_clock;
    CPU &m_cpu;
    Pheriph **m_pheripherials = nullptr;
    int m_pheriphCount = 0;

private:
    void printError(const char* fmt, ...);

private:
    FQ m_fq;
    std::function<void(const std::string&)> m_error_callback = nullptr;
};

#endif // _COMPUTER_H_