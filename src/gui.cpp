#include "gui.h"


GUI::GUI(const CPU_Status &status, Clock &clock, CPU& cpu)
    : m_CPUStatus(status), m_clock(clock), m_cpu(cpu)
{
    m_pheripherials = m_cpu.mem.getPheripherials(&m_pheriphCount);
}

GUI::~GUI()
{
    delete[] m_pheripherials;
}

bool GUI::windowClosed()
{
    return end;
}