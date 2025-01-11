#include "Emulator.h"
#include "FilePath.h"

CPU cpu;

void cycle(void)
{
    cpu.cycle();
}

Emulator::Emulator()
    : m_fq(1000000), m_clock(cycle), m_cpu(cpu), m_debuggerData(), m_gui(cpu.getStatus(), m_clock, cpu, m_debuggerData.SymbolData)
{
    m_clock.setHZ(m_fq.HZ);
}

int Emulator::init()
{
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
    if(isExtEqual(filename, "json"))
    {
        m_gui.projectFileName = filename;
        m_gui.sourceFileNames.clear();
        m_clock.setStatus(false);
        m_gui.NewProjectOpened = false;
        if(m_debuggerData.init(filename))
        {
            m_gui.displayError("Failed to Open project:\n%s", m_gui.projectFileName.c_str());
            return 1;
        }
        m_gui.sourceFileNames = m_debuggerData.getFileNames();
        filename = m_gui.projectPath + '/' + m_gui.sourceFileNames[0];
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
    start();
    while (isRunning())
    {
        m_gui.mainLoop();
        if(!m_clock.getStatus())
        {
            m_gui.currentPosition = m_debuggerData.getBreakpoint(m_cpu.getStatus().PC.addr);
        }
        if (m_gui.NewProjectOpened)
        {
            if(load(m_gui.projectFileName))
            {
                continue;
            }
            m_cpu.startExec();
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