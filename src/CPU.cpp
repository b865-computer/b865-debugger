#include "CPU.h"
#include "SignalTables.h"
#include <fstream>
#include "intelhex.h"

std::vector<std::string> IO_regNames =
    {
        "DDR0",
        "PORT0",
        "DDR1",
        "PORT1",
        "DDR2",
        "PORT2",
        "DDR3",
        "PORT3",
};

Pheriph IO(0x100, "IO", IO_regNames);

#define MEMORY_REGION_COUNT 4

MEMMAP MemMap[MEMORY_REGION_COUNT] = {
    MEMMAP(MEMMAP::REGION_TYPE::rom, 0x8000, 0x8000),
    MEMMAP(MEMMAP::REGION_TYPE::ram, 0, 0x0200),
    MEMMAP(&IO, 0x0200, 0x100),
    MEMMAP(MEMMAP::REGION_TYPE::ram, 0x0300, 0x7D00),
};

CPU::CPU()
    : mem(MemMap, MEMORY_REGION_COUNT)
{
}

void CPU::init()
{
    A = B = PC = 0;
    PC = 0x8000;
    signals.val = 0;
    AdrState = false;
    signals.HLT = 1;
}

int CPU::loadProgram(uint8_t *newprogram, uint32_t len)
{
    mem.cpy(0x8000, newprogram, (len > 0x8000 ? 0x8000 : len));
    return 0;
}
int CPU::loadProgramFromFile(std::string filename)
{
    uint32_t len = 0x8000;
    int rval = 0;
    std::ifstream file(filename, std::ios::binary | std::ios::in);
    if (!file.is_open())
    {
        fprintf(stderr, "Error: unable to open file %s\n", filename.c_str());
        return 1;
    }
    uint8_t *program = new uint8_t[len];

    if (isExtEqual(filename, "hex"))
    {
        intelhex::hex_data data;
        data.read(file);
        for(auto i = data.begin(); i != data.end(); i++)
        {
            if(i->first + i->second.size() > (len * 2))
            {
                fprintf(stderr, "Error: file %s is too large\n", filename.c_str());
                break;
            }
            for(int j = 0; j < i->second.size(); j++)
            {
                if(i->first + j < len)
                {
                    fprintf(stderr, "Error: regions under adress 0x8000 don't get imported.\n");
                    break;
                }
                program[i->first + j - len] = i->second[j];
            }
        }
    }
    else
    {
        std::streampos begin, end;
        begin = file.tellg();
        file.seekg(0, std::ios::end);
        end = file.tellg();
        file.seekg(0);
        uint32_t len = end - begin;
        if (len > 0x8000)
        {
            fprintf(stderr, "Error: file size exceeds 0x8000 bytes\n");
            rval = 1;
            goto END;
        }
        file.read((char *)program, len);
    }

    loadProgram(program, len);

END:
    delete[] program;
    file.close();
    return rval;
}
void CPU::startExec()
{
    signals.HLT = 0;
    IR0 = IR1 = A = B = flags.val = InsCycle = AdrCycle = 0;
    PC = 0;
    for (int i = 0; i < 8; i++)
    {
        registers[i] = 0;
    }
    started = false;
}

void CPU::stopPheripherials()
{
    mem.stopPheripherials();
}

const CPU_Status &CPU::getStatus()
{
    return *this;
}

void CPU::cycle()
{
    if (signals.HLT)
    {
        return;
    }
    if (AdrState)
    {
        signals.val = AdrSignalTable[AddrMode][AdrCycle++];
    }
    else
    {
        signals.val = InsSignalTable[IR0 & 0x3F][InsCycle++];
        if(signals.FLAG_MASK)
        {
            IR0 = ((IR0 & 0x1F) | ((IR0 >> 5) == flags.val ? 1 : 0));
            signals.val = InsSignalTable[IR0 & 0x3F][InsCycle - 1];
        }
    }
    executeSignals();
}

void CPU::executeSignals()
{
    if(signals.IMC && (AddrMode == 6))
    {
        signals.CE = 1;
    }

    RI = signals.RIO_SE ? ((IR0 & 0xE0) >> 5) : (IR1 & 0x07);
    RO = signals.RIO_SE ? (IR1 & 0x07) : ((IR0 & 0xE0) >> 5);
    ALU_OP = IR0 & 0x07;
    int regNum = signals.RO_B0 + (signals.RO_B1 << 1) + (signals.RO_B2 << 2);
    uint8_t DBus = getRegOut(regNum, RO);

    if (signals.IR1I)
    {
        IR1 = mem.get(MAR.addr);
    }
    if (InsCycle == 1 && !AdrModeSelect && started)
    {
        IR0 = mem.get(MAR.addr);
    }
    AddrMode = (!AdrModeSelect) ? (IR1 & 0xF) : ((IR1 & 0xF0) >> 4);
    if (signals.ROM_SE && (AddrMode != 6))
    {
        AdrCycle = 0;
        AdrState = AdrState ? false : true;
    }

    if (signals.CE)
    {
        PC.addr++;
    }

    uint16_ADDR ABus;
    if (signals.SPO_A)
    {
        ABus = (registers[REGISTERS_BANK::SP_IDX] + 0x100);
    }
    else if (signals.ARO)
    {
        ABus = AR | (DBus << 8);
    }
    else
    {
        ABus = PC.addr;
    }

    if (signals.MI)
    {
        mem.set(MAR.addr, DBus);
    }
    if (signals.AM_SE)
    {
        AdrModeSelect = true;
    }
    if (signals.ARI)
    {
        AR = DBus;
    }
    if (signals.AI)
    {
        A = DBus;
    }
    if (signals.BI)
    {
        B = DBus;
    }
    if (signals.PCI)
    {
        PC = ABus;
    }
    if (signals.RI_EN)
    {
        registers[RI] = DBus;
    }
    if (signals.SP_DEC)
    {
        registers[REGISTERS_BANK::SP_IDX]--;
    }
    if (signals.SP_INC)
    {
        registers[REGISTERS_BANK::SP_IDX]++;
    }
    if (signals.X_DEC)
    {
        registers[REGISTERS_BANK::X_IDX]--;
    }
    if (signals.X_INC)
    {
        registers[REGISTERS_BANK::X_IDX]++;
    }
    MAR = ABus;
    if (signals.INS_END)
    {
        InsCycle = AdrCycle = 0;
        AdrModeSelect = false;
        started = true;
    }
}

uint8_t CPU::getRegOut(uint8_t regNum, uint8_t bankRegNum)
{
    switch (regNum)
    {
    case REGISTERS_STATE::A_O_IDX:
        return A;
        break;
    case REGISTERS_STATE::R_O_IDX:
        return registers[bankRegNum];
        break;
    case REGISTERS_STATE::S_O_IDX:
        return calcALUOut();
        break;
    case REGISTERS_STATE::X_O_IDX:
        return registers[REGISTERS_BANK::X_IDX];
        break;
    case REGISTERS_STATE::Y_O_IDX:
        return registers[REGISTERS_BANK::Y_IDX];
        break;
    case REGISTERS_STATE::M_O_IDX:
        return mem.get(MAR.addr);
        break;
    case REGISTERS_STATE::PCH_O_IDX:
        return PC.H;
        break;
    case REGISTERS_STATE::PCL_O_IDX:
        return PC.L;
        break;
    default:
        break;
    }
    return 0;
}

uint8_t CPU::calcALUOut()
{
    // RIO_SE is the same as SUB for saving space int the real cpu.
    int OP = signals.ALU_OP ? ALU_OP : signals.RIO_SE;
    // Operand 2 means subtraction (0 = add), so then the carry defaults to 1.
    // Other cases doesn't matter (eg. 3 = XOR) , because they don't use carry.
    int carry = signals.CA_SE ? flags.carry : ((OP & 2) >> 1);
    int B_val = signals.DBS ? B : 1;
    uint16_t val = 0;
    if (signals.ROR)
    {
        uint8_t val = (A >> 1) + (carry << 7);
        carry = A & 1;
    }
    else
    {
        switch (OP)
        {
        case 0:
            val = (A + B_val + carry);
            break;
        case 1:
            val = ~(A ^ B_val);
            break;
        case 2:
            val = (A + (B_val ^ 0xFF) + carry);
            break;
        case 3:
            val = (A ^ B_val);
            break;
        case 4:
            val = (A | B_val);
            break;
        case 5:
            val = ~(A | B_val);
            break;
        case 6:
            val = ~(A & B_val);
            break;
        case 7:
            val = (A & B_val);
            break;
        default:
            val = 0;
            break;
        }
        carry = (val & 0x100) ? 1 : 0;
    }
    if (signals.FI)
    {
        flags.carry = carry;
        flags.zero = (val) ? 1 : 0;
        flags.negative = (carry == 0 && flags.zero == 0 && ALU_OP & 2) ? 1 : 0;
    }
    return val;
}

void CPU::cycle_ins_level()
{
    if(m_cycle)
    {
        m_cycle--;
    }
    switch(IR0)
    {
    // TODO: Add more instructions
    case 0: // reset
        registers[X_IDX] = 0xFF;
        PC.addr = mem.get(0xFFFE) << 8 | mem.get(0xFFFF);
        IR0 = mem.get(PC.addr++);
        m_cycle = 6;
        break;
    default:
        break;
    }
}