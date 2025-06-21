#include "Emulator.h"
#include "Utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <cstdarg>


CPU cpu;

void cycle(void)
{
    cpu.cycle();
}
void cycle_ins_level(void)
{
    cpu.cycle_ins_level();
}

Emulator::Emulator(std::function<void(const std::string&)> error_callback)
    : m_debuggerData(), m_clock(cycle), m_cpu(cpu), m_fq(1000000), m_error_callback(error_callback)
{
    m_clock.setHZ(m_fq.HZ);
    m_pheripherials = nullptr;
    m_pheriphCount = 0;
}

Emulator::~Emulator()
{
    if (m_pheripherials)
    {
        delete[] m_pheripherials;
    }
}

int Emulator::init()
{
    CdbgExpr::SymbolDescriptor::data = this;
    m_cpu.init();
    m_clock.init();
    m_clock.m_cycle_func = cycle;
    m_pheripherials = m_cpu.mem.getPheripherials(&m_pheriphCount);
    return 0;
}

int Emulator::load(std::string filename)
{
    if(isExtEqual(filename, "b865"))
    {
        m_clock.setStatus(false);
        if(m_debuggerData.init(filename))
        {
            printError("Failed to Open project:\n%s", filename);
            return 1;
        }
        std::string path = getPath(filename);
        filename = path + "/" + m_debuggerData.getBinFile();
    }
    if(m_cpu.loadProgramFromFile(filename))
    {
        return 1;
    }
    m_cpu.startExec();
    return 0;
}

int Emulator::load(std::vector<uint8_t> &programData)
{
    return m_cpu.loadProgram(programData.data(), programData.size());
}

int Emulator::main()
{
    return 0;
}
void Emulator::start()
{
    m_clock.setStatus(true);
}

void Emulator::stop()
{
    m_clock.setStatus(false);
}

void Emulator::terminate()
{
    m_clock.terminate();
    m_cpu.stopPheripherials();
}

std::chrono::nanoseconds Emulator::getRunTime_ns()
{
    return m_clock.getRunTime_ns();
}

void Emulator::setInsLevel(bool insLevel)
{
    auto prev = m_clock.getStatus();
    m_clock.setStatus(false);
    m_clock.m_cycle_func = insLevel ? cycle_ins_level : cycle;
    m_clock.setStatus(prev);
}

bool Emulator::pausedAtBreakpoint()
{
    if (m_cpu.stoppedAtBreakpoint)
    {
        pause();
    }
    return m_cpu.stoppedAtBreakpoint;
}

void Emulator::pause()
{
    m_cpu.stoppedAtBreakpoint = true;
    stop();
}

bool Emulator::clockRunning()
{
    return m_clock.getStatus();
}

void Emulator::continue_exec()
{
    m_cpu.stoppedAtBreakpoint = false;
}

void Emulator::printError(const char *fmt, ...)
{
    if (m_error_callback == nullptr)
    {
        return;
    }
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    m_error_callback(std::string(buffer));
}

CdbgExpr::SymbolDescriptor Emulator::getSymbol(const std::string &name)
{
    auto it = std::find_if(m_debuggerData.globalScope.begin(), m_debuggerData.globalScope.end(), 
        [name](SymbolVector::value_type symbol)
        {
            return symbol.name == name;
        });
    if (it != m_debuggerData.globalScope.end())
    {
        return *it;
    }
    return CdbgExpr::SymbolDescriptor();
}

uint8_t Emulator::getByte(uint64_t address)
{
    if (address < 0x10000)
    {
        return m_cpu.mem.get(address);
    }
    return 0;
}

void Emulator::setByte(uint64_t address, uint8_t value)
{
    if (address < 0x10000)
    {
        m_cpu.mem.set(address, value);
    }
    return;
}

uint8_t Emulator::CTypeSize(CdbgExpr::CType type)
{
    switch (type.type)
    {
    case CdbgExpr::CType::Type::DOUBLE:
    case CdbgExpr::CType::Type::LONGLONG:
        return 8;

    case CdbgExpr::CType::Type::LONG:
    case CdbgExpr::CType::Type::FLOAT:
        return 4;
    
    case CdbgExpr::CType::Type::INT:
        return 2;
    
    case CdbgExpr::CType::Type::SHORT:
    case CdbgExpr::CType::Type::POINTER:
        return 2;
    
    case CdbgExpr::CType::Type::CHAR:
    case CdbgExpr::CType::Type::BOOL:
    case CdbgExpr::CType::Type::VOID_type:
        return 1;
    
    default:
        break;
    }
    return 0;
}

uint64_t Emulator::getStackPointer()
{
    return m_cpu.getStatus().registers[REGISTERS_BANK::SP_IDX];
}

uint8_t Emulator::getRegContent(uint8_t regNum)
{
    if (regNum >= 8)
    {
        return 0;
    }
    return m_cpu.getStatus().registers[regNum];
}

void Emulator::setRegContent(uint8_t regNum, uint8_t val)
{
    if (regNum >= 8)
    {
        return;
    }
    m_cpu.setReg(regNum, val);
}

size_t Emulator::getCurrentBreakPointId()
{
    auto it = std::find_if(breakpoints.breakpoints.begin(), breakpoints.breakpoints.end(), [this](const Breakpoint& b)
    {
        return m_cpu.savedPC == b.addr;
    });
    if (it != breakpoints.breakpoints.end())
    {
        return it->id;
    }
    return 0;
}

void Emulator::addBreakpoint(const std::vector<std::string> &args)
{
    auto bp = breakpoints.addBreakpoint(args, &m_debuggerData.data);
    m_cpu.breakpoints.insert(bp.addr);
}

void Emulator::delBreakpoint(const std::vector<std::string> &args)
{
    breakpoints.delBreakpoint(args);
    std::unordered_set<uint16_t> addrSet(breakpoints.addresses.begin(), breakpoints.addresses.end());
    m_cpu.setBreakpoints(addrSet);
}