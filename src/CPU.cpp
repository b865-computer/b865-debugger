#include "CPU.h"
#include "SignalTables.h"

void CPU::init()
{
    AC = A = B = IR0 = IR1 = SP = 0;
    ADDRESS.H = 0x80;
    ADDRESS.L = 0x00;
    PC.addr = 0x8000;
    isComputing = false;
    isFeching = false;
    isFechingData = false;
    signals.val = 0;
    signals.HLT = 1;
}

void CPU::cycle()
{
    if (signals.HLT)
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
            Cycle = 0;
        }
    }
    else if (isFechingData)
    {
        signals.val = AddrModeTable[addrMode][Cycle];
        if(!signals.val)
        {
            isFechingData = false;
        }
        if (!isFechingData)
        {
            isComputing = true;
            Cycle = 0;
        }
        Cycle++;
    }
    if (isComputing)
    {
        signals.val = ISTable[IR0][Cycle];
        if (!signals.val)
        {
            isComputing = false;
        }
        if (!isComputing)
        {
            isFeching = true;
            Cycle = 0;
            Istate = 0;
            Cycle = 0;
        }
        Cycle++;
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
    if (signals.RO_EN)
    {
        DBus = getRegister(signals.RO_B0);
    }
    else if (signals.M_O)
    {
        DBus = mem.get();
    }
    else if (signals.ALU_OUT)
    {
        signals.ALU_B0 = IR0 & 0xF;
        DBus = ALU(signals.CA_SE == 1 ? flags.carry : signals.ALU_B0, signals.ALU_B0, signals.ROR);
    }
    if (signals.RI_EN)
    {
        getRegister(signals.RO_B0) = DBus;
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
        if (A - B < 0 && signals.ALU_B0 == 1)
        {
            flags.negative = 1;
        }
        if (A + B > 255)
        {
            flags.carry = 1;
        }
    }
    else if (signals.M_O)
    {
        mem.set(DBus);
    }
    if (signals.SP_INC)
    {
        SP++;
    }
    if (signals.SP_DEC)
    {
        SP--;
    }
    if (signals.SP_O)
    {
        ABus.addr = SP + 0x1000;
    }
    if (signals.PC_INC)
    {
        PC.addr++;
    }
    if (signals.PC_O)
    {
        ABus.addr = PC.addr;
    }
    if (signals.PC_I)
    {
        PC.addr = ABus.addr;
    }
    if (signals.X_INC)
    {
        X++;
    }
    if (signals.X_DEC)
    {
        X--;
    }
    if (signals.Y_INC)
    {
        Y++;
    }
    if (signals.Y_DEC)
    {
        Y--;
    }
    if (signals.IRI0)
    {
        IR0 = mem.get();
    }
    if (signals.IRI1)
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

uint8_t CPU::ALU(uint8_t carry, uint8_t OP, bool shift)
{
    if(shift)
    {
        uint8_t val = (A >> 1) + (carry << 7);
        carry = A & 1;
        return val;
    }
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
