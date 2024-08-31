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
        byte L;
        byte H;
    };
};

struct CPU_State
{
    byte AC, A, B, IR0, IR1, SP;
    uint16_ADDR PC, ADDRESS;
    byte Istate;
};

union Signals
{
    struct
    {
        uint8_t HLT : 1;
        struct RI
        {
            uint8_t EN : 1;
            union
            {
                uint8_t R : 3;
                struct
                {
                    uint8_t B2 : 1;
                    uint8_t B1 : 1;
                    uint8_t B0 : 1;
                };
            };
        };
        struct RO
        {
            uint8_t EN : 1;
            union
            {
                uint8_t R : 3;
                struct
                {
                    uint8_t B2 : 1;
                    uint8_t B1 : 1;
                    uint8_t B0 : 1;
                };
            };
        };
        uint8_t FI : 1;
        struct M
        {
            uint8_t I : 1;
            uint8_t O : 1;
        };
        struct ALU
        {
            uint8_t OUT : 1;
            union
            {
                uint8_t OP : 3;
                struct
                {
                    uint8_t B2 : 1;
                    uint8_t B1 : 1;
                    uint8_t B0 : 1;
                };
            };
        };
        struct PC
        {
            uint8_t INC : 1; // CE
            uint8_t I : 1;
            uint8_t O : 1;
        };
        struct PC
        {
            uint8_t INC : 1;
            uint8_t DEC : 1;
            uint8_t O : 1;
        };
        uint8_t CA_SE : 1; // Carry slelect
        struct X
        {
            uint8_t INC : 1;
            uint8_t DEC : 1;
        };
        struct Y
        {
            uint8_t INC : 1;
            uint8_t DEC : 1;
        };
        struct IR
        {
            uint8_t I0 : 1;
            uint8_t I1 : 1;
        };
        uint8_t reserved : 3;
    };
    uint32_t val;
};

class CPU : public CPU_State
{
public:
    void init();
    void cycle();

private:
    void executeSignals();

private:
    MEM mem;
    Signals signals;
    byte Istate = 0;
    bool isFeching = true;
    bool isFechingData = false;
    bool isComputing = false;
    int fechCycle = 0;
    byte addrMode = 0, addrMode2 = 0, RI = 0, RO = 0;
};

#endif // _CPU_H_