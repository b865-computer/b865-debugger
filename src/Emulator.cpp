#include "Emulator.h"

CPU cpu;

void cycle(void)
{
    cpu.cycle();
}

Emulator::Emulator()
: m_fq(1000000), m_clock(cycle), m_cpu(cpu)
{
    m_clock.setHZ(m_fq.HZ);
}

void Emulator::init()
{
    m_gui.init();
    m_cpu.init();
}

int Emulator::load(std::string filename)
{
    return m_cpu.loadProgramFromFile(filename);
}

int Emulator::load(std::vector<uint8_t> &programData)
{
    return m_cpu.loadProgram(programData.data(), programData.size());
}

void Emulator::start()
{
    m_cpu.startExec();
    m_clock.start();
}

void Emulator::stop()
{
    m_clock.stop();
    m_cpu.stopPheripherials();
}

std::chrono::nanoseconds Emulator::getRunTime_ns()
{
    return m_clock.getRunTime_ns();
}

bool Emulator::isRunning()
{
    if(m_clock.getRunTimeCycles_ns() < 10000000000)
    {
        return true;
    }
    else
    {
        stop();
        fprintf(stdout,"%i cycles in %f seconds, %iHz, target: %iHz\n", m_clock.getCycles(),
            (double)m_clock.getRunTime_ns().count() / 1e9,
            (int)(m_clock.getCycles() / ((double)m_clock.getRunTime_ns().count() / 1e9)), m_fq.HZ);
    }
    return false;
}

int Emulator::exitCode()
{
    return m_exitCode;
}
