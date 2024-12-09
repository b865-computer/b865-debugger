#include "Emulator.h"

CPU cpu;

void cycle(void)
{
    cpu.cycle();
}

Emulator::Emulator()
: m_fq(1000000), m_clock(cycle), m_cpu(cpu), m_gui(cpu.getStatus())
{
    m_clock.setHZ(m_fq.HZ);
}

int Emulator::init()
{
    m_cpu.init();
    m_clock.init();
    return m_gui.init();
}

int Emulator::load(std::string filename)
{
    return m_cpu.loadProgramFromFile(filename);
}

int Emulator::load(std::vector<uint8_t> &programData)
{
    return m_cpu.loadProgram(programData.data(), programData.size());
}

int Emulator::main()
{
    start();
    while(isRunning())
    {
        m_gui.mainLoop();
    }
    return 0;
}

void Emulator::start()
{
    m_cpu.startExec();
    m_clock.setStatus(true);
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
    if(!m_gui.windowClosed())
    {
        return true;
    }
    else
    {
        stop();
        // fprintf(stdout,"%i cycles in %f seconds, %iHz, target: %iHz\n", m_clock.getCycles(),
        //     (double)m_clock.getRunTime_ns().count() / 1e9,
        //     (int)(m_clock.getCycles() / ((double)m_clock.getRunTime_ns().count() / 1e9)), m_fq.HZ);
    }
    return false;
}