#pragma once

#ifndef _CPU_H_
#define _CPU_H_

#include "Common.h"
#include "MEM.h"
#include <unordered_set>

union uint16_ADDR
{
    uint16_t addr;
    struct
    {
        uint8_t L;
        uint8_t H;
    };
    inline uint16_t &operator=(uint16_t value)
    {
        addr = value;
        return this->addr;
    }
};

enum REGISTERS_BANK
{
    AC_IDX = 0,
    X_IDX,
    Y_IDX,
    SP_IDX,
    R0_IDX,
    R1_IDX,
    R2_IDX,
    R3_IDX,
};

enum REGISTERS_STATE
{
    A_O_IDX = 0,
    R_O_IDX,
    S_O_IDX,
    X_O_IDX,
    Y_O_IDX,
    M_O_IDX,
    PCH_O_IDX,
    PCL_O_IDX,
};

union CPU_Signals
{
    struct
    {
        uint32_t SPO_A : 1;
        uint32_t X_INC : 1;
        uint32_t MI : 1;
        uint32_t FLAG_MASK : 1;
        uint32_t res_3 : 1;
        uint32_t ROR : 1;
        uint32_t X_DEC : 1;
        uint32_t SP_INC : 1;
        uint32_t SP_DEC : 1;
        uint32_t res_2 : 1;
        uint32_t RI_EN : 1;
        uint32_t ALU_OP : 1;
        uint32_t IR1I : 1;
        uint32_t CA_SE : 1;
        uint32_t DBS : 1;
        uint32_t PCI : 1;
        uint32_t RO_B0 : 1;
        uint32_t RO_B1 : 1;
        uint32_t RO_B2 : 1;
        uint32_t BI : 1;
        uint32_t AI : 1;
        uint32_t ARO : 1;
        uint32_t ARI : 1;
        uint32_t res_1 : 1;
        uint32_t FI : 1;
        uint32_t CE : 1;
        uint32_t IMC : 1;
        uint32_t RIO_SE : 1;
        uint32_t HLT : 1;
        uint32_t AM_SE : 1;
        uint32_t ROM_SE : 1;
        uint32_t INS_END : 1;
    };
    uint32_t val;
};

class CPU_Status
{
public:
    bool started = false;
    uint8_t registers[8];
    uint8_t A, B, IR0, IR1, AR;
    uint16_ADDR PC, MAR;
    CPU_Signals signals;
    bool AdrState = false;
    bool AdrModeSelect = false;
    int InsCycle = 0;
    int AdrCycle = 0;
    uint8_t RI = 0, RO = 0, ALU_OP = 0;
    int AddrMode;
    union
    {
        struct
        {
            uint8_t carry : 1;
            uint8_t zero : 1;
            uint8_t negative : 1;
        };
        uint8_t val;
    } flags;
};

class CPU : private CPU_Status
{
public:
    CPU();
    void init();
    void cycle();
    void cycle_ins_level();
    int loadProgram(uint8_t *newprogram, uint32_t len);
    int loadProgramFromFile(std::string filename);
    void startExec();
    void stopPheripherials();
    const CPU_Status &getStatus();
    void setReg(uint8_t regNum, uint8_t val);
    void setBreakpoints(const std::unordered_set<uint16_t>& _breakpoints);

public:
    MEMORY mem;
    bool stoppedAtBreakpoint = false;
    uint16_t savedPC = 0;
    std::unordered_set<uint16_t> breakpoints;

private:
    void executeSignals();
    uint8_t getRegOut(uint8_t regNum, uint8_t bankRegNum);
    uint8_t calcALUOut();

private:
    int m_cycle = 0;
};

#endif // _CPU_H_