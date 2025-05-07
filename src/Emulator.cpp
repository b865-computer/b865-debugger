#include "Emulator.h"
#include "Utils.h"

CPU cpu;

void cycle(void)
{
    cpu.cycle();
}
void cycle_ins_level(void)
{
    cpu.cycle_ins_level();
}

Emulator::Emulator()
    : m_debuggerData(), m_clock(cycle), m_cpu(cpu), m_gui(*this, cpu.getStatus(), m_clock, cpu), m_fq(1000000)
{
    m_clock.setHZ(m_fq.HZ);
}

int Emulator::init()
{
    CdbgExpr::SymbolDescriptor::data = this;
    m_cpu.init();
    m_clock.init();
    return m_gui.init();
}

int Emulator::load(std::string filename, std::string path)
{
    if(path.size() != 0)
    {
        filename = path + "/" + filename;
        m_gui.projectPath = path;
    }
    else
    {
        path = m_gui.projectPath = getPath(filename);
    }
    if(isExtEqual(filename, "b865"))
    {
        m_gui.projectFileName = filename;
        m_clock.setStatus(false);
        m_gui.NewProjectOpened = false;
        if(m_debuggerData.init(filename))
        {
            m_gui.displayError("Failed to Open project:\n%s", m_gui.projectFileName.c_str());
            return 1;
        }
        std::string path = getPath(filename);
        filename = path + "/" + m_debuggerData.getBinFile();
    }
    if(m_cpu.loadProgramFromFile(filename))
    {
        m_gui.displayError("Failed to load program from file:\n%s", filename.c_str());
        return 1;
    }
    return 0;
}

int Emulator::load(std::vector<uint8_t> &programData)
{
    return m_cpu.loadProgram(programData.data(), programData.size());
}

int Emulator::main()
{
    std::string outputLines;
    m_gui.ConsoleText = &outputLines;
    M_PROCESS_OUT buildProcessOut = nullptr;
    M_PROCESS buildProcess;
    bool buildProcessRunning = false;
    bool ins_level = false;
    std::string buildCmd;
    start();
    while (isRunning())
    {
        m_gui.main();
        if(!m_clock.getStatus())
        {
            m_gui.currentPosition = m_debuggerData.getPosition(m_cpu.getStatus().PC.addr - 1);
        }
        if(m_gui.ins_level && !ins_level)
        {
            ins_level = true;
            m_clock.m_cycle_func = cycle_ins_level;
        }
        else if(!m_gui.ins_level && ins_level)
        {
            ins_level = false;
            m_clock.m_cycle_func = cycle;
        }
        if (m_gui.NewProjectOpened)
        {
            if(load(m_gui.projectFileName))
            {
                continue;
            }
            m_cpu.startExec();
        }
        if(m_gui.buildRunning && !buildProcessRunning)
        {
            buildCmd = "make";
            outputLines.clear();
            outputLines += buildCmd + "\n";
            buildProcessOut = startProgram(m_gui.projectPath, buildCmd, buildProcess);
            if(buildProcess == M_PROCESS_INVALID || buildProcessOut == M_PROCESS_INVALID)
            {
                m_gui.displayError("Failed to start build process");
                m_gui.buildRunning = false;
                continue;
            }
            buildProcessRunning = true;
        }
        if (buildProcessRunning)
        {
            buildProcessRunning = pollProgramOutput(buildProcessOut, outputLines);
            if(!buildProcessRunning)
            {
                unsigned long exitCode;
                if((exitCode = programExitCode(buildProcess, buildProcessOut, &buildProcessRunning)))
                {
                    m_gui.displayError("Build process exited with code: %d", exitCode);
                }
                if(!buildProcessRunning)
                {
                    outputLines += ("Process exited with code: " + std::to_string(exitCode) + "\n");
                }
            }
            m_gui.buildRunning = buildProcessRunning;
        }
    }
    return 0;
}
void Emulator::start()
{
    m_cpu.startExec();
    m_clock.setStatus(false);
}

void Emulator::stop()
{
    m_clock.terminate();
    m_cpu.stopPheripherials();
    m_gui.terminate();
}

std::chrono::nanoseconds Emulator::getRunTime_ns()
{
    return m_clock.getRunTime_ns();
}

bool Emulator::isRunning()
{
    if (!m_gui.windowClosed())
    {
        return true;
    }
    else
    {
        stop();
    }
    return false;
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
    case CdbgExpr::CType::Type::VOID:
        return 1;
    
    default:
        break;
    }
    return 0;
}