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
    if(isExtEqual(filename, "b865"))
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
        filename = m_gui.sourceFileNames[0];
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
    M_PROCESS_OUT buildProcessOut;
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
            // Check if the PC points to the instruction, PC is always 1 byte ahead of the last memory address.
            if((m_cpu.mem.get(m_cpu.getStatus().PC.addr - 1) & 0x1F) == (m_cpu.getStatus().IR0 & 0x1F))
            {
                m_gui.currentPosition = m_debuggerData.getBreakpoint(m_cpu.getStatus().PC.addr - 1);
            }
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