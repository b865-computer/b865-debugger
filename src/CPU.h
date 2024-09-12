#pragma once

#ifndef _CPU_H_
#define _CPU_H_

#include <iostream>
#include "Computer.h"
#include "MEM.h"


union uint16_ADDR
{
    uint16_t addr;
    struct
    {
        uint8_t L;
        uint8_t H;
    };
};

struct CPU_State
{
    uint8_t AC, A, B, IR0, IR1, SP, X, Y;
    uint16_ADDR PC, ADDRESS;
    uint8_t Istate;
    bool Halted;
};

union Signals
{
    struct
    {
        uint32_t HLT : 1;

        uint32_t RI_EN : 1;

        uint32_t RI_B2 : 1;
        uint32_t RI_B1 : 1;
        uint32_t RI_B0 : 1;

        uint32_t RO_EN : 1;

        uint32_t RO_B2 : 1;
        uint32_t RO_B1 : 1;
        uint32_t RO_B0 : 1;

        uint32_t FI : 1;

        uint32_t M_I : 1;
        uint32_t M_O : 1;

        uint32_t ALU_OUT : 1;

        uint32_t ALU_B2 : 1;
        uint32_t ALU_B1 : 1;
        uint32_t ALU_B0 : 1;

        uint32_t PC_INC : 1; // CE
        uint32_t PC_I : 1;
        uint32_t PC_O : 1;

        uint32_t SP_INC : 1;
        uint32_t SP_DEC : 1;
        uint32_t SP_O : 1;

        uint32_t CA_SE : 1; // Carry select

        uint32_t X_INC : 1;
        uint32_t X_DEC : 1;

        uint32_t Y_INC : 1;
        uint32_t Y_DEC : 1;

        uint32_t IRI0 : 1;
        uint32_t IRI1 : 1;

        uint32_t ROR : 1;
        
        uint32_t reserved : 2;
    };
    uint32_t val;
};

class CPU : public CPU_State
{
public:
    void init();
    void cycle();

public:
    void executeSignals();
    uint8_t &getRegister(uint8_t regNum);
    uint8_t ALU(uint8_t carry, uint8_t OP, bool shift);

public:
    MEM mem;
    Signals signals;
    uint8_t Istate = 0;
    bool isFeching = true;
    bool isFechingData = false;
    bool isComputing = false;
    int Cycle = 0;
    uint8_t addrMode = 0, addrMode2 = 0, RI = 0, RO = 0;
    struct
    {
        uint8_t carry : 1;
        uint8_t zero : 1;
        uint8_t negative : 1;
    } flags;
};

#endif // _CPU_H_