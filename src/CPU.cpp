#include "CPU.h"

uint32_t AddrModeTable[16][16];

void CPU::init()
{
    AC = A = B = IR0 = IR1 = SP = 0;
    ADDRESS.H = 0x80;
    ADDRESS.L = 0x00;
    PC.addr = 0x8000;
    isComputing = false;
    isFeching = false;
    isFechingData = false;
    Halted = false;
}

void CPU::cycle()
{
    if (Halted)
    {
        return;
    }
    if (isFeching)
    {
        if (Istate == 0)
        {
            IR0 = mem.get();
            PC.addr++;
            mem.setAddress(PC.addr);
        }
        else
        {
            IR1 = mem.get();
            PC.addr++;
            mem.setAddress(PC.addr);
            isFeching = false;
            isFechingData = true;
            fechCycle = 0;
        }
    }
    else if (isFechingData)
    {
        signals.val = AddrModeTable[addrMode][fechCycle];
        if (!isFechingData)
        {
            isComputing = true;
        }
        fechCycle++;
    }
    if (isComputing)
    {
    }
    Istate++;
}

void CPU::executeSignals()
{
    byte DBus;
    uint16_ADDR ABus;
    if (signals.HLT)
    {
        Halted = true;
    }
    if (signals.RO.EN)
    {
        DBus = getRegister(signals.RO.R);
    }
    else if (signals.M.O)
    {
        DBus = mem.get();
    }
    else if (signals.ALU.OUT)
    {
        signals.ALU.OP = IR0 & 0xF;
        DBus = ALU(signals.CA_SE == 1 ? flags.carry : signals.ALU.B0, signals.ALU.OP);
    }
    if (signals.RI.EN)
    {
        getRegister(signals.RO.R) = DBus;
    }
    if (signals.FI)
    {
        flags.carry = 0;
        flags.zero = 0;
        flags.negative = 0;
        if (A + B == 0)
        {
            flags.zero = 1;
        }
        if (A - B < 0 && signals.ALU.OP == 1)
        {
            flags.negative = 1;
        }
        if (A + B > 255)
        {
            flags.carry = 1;
        }
    }
    else if (signals.M.O)
    {
        mem.set(DBus);
    }
    if(signals.SP.INC)
    {
        SP++;
    }
    if(signals.SP.DEC)
    {
        SP--;
    }
    if(signals.SP.O)
    {
        ABus.addr = SP + 0x1000;
    }
    if(signals.PC.INC)
    {
        PC.addr++;
    }
    if(signals.PC.O)
    {
        ABus.addr = PC.addr;
    }
    if(signals.PC.I)
    {
        PC.addr = ABus.addr;
    }
    if(signals.X.INC)
    {
        X++;
    }
    if(signals.X.DEC)
    {
        X--;
    }
    if(signals.Y.INC)
    {
        Y++;
    }
    if(signals.Y.DEC)
    {
        Y--;
    }
    if(signals.IR.I0)
    {
        IR0 = mem.get();
    }
    if(signals.IR.I1)
    {
        IR1 = mem.get();
    }
    mem.setAddress(PC.addr);
}

uint8_t &CPU::getRegister(uint8_t regNum)
{
    uint8_t *reg;
    switch (regNum)
    {
    case 0:
        reg = &A;
        break;
    case 1:
        reg = &X;
        break;
    case 2:
        reg = &Y;
        break;
    case 3:
        reg = &SP;
        break;
    case 4:
        reg = &(PC.L);
        break;
    case 5:
        reg = &(PC.H);
        break;
    case 6:
        reg = &AC;
        break;
    case 7:
        reg = &B;
        break;
    default:
        break;
    }
    return (*reg);
}

uint8_t CPU::ALU(uint8_t carry, uint8_t OP)
{
    switch (OP)
    {
    case 0:
        return A + B + carry;
    case 1:
        return A + (B ^ 0xFF) + carry;
    case 2:
        return A ^ B;
    case 3:
        return A | B;
    case 4:
        return ~A;
    case 5:
        return ~(A | B);
    case 6:
        return ~(A & B);
    case 7:
        return A & B;
    default:
        return 0;
    }
    return 0;
}
