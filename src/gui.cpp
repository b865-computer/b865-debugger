#include "gui.h"

GUI::GUI(const CPU_Status &status, Clock &clock, CPU &cpu)
    : m_CPUStatus(status), m_clock(clock), m_cpu(cpu)
{
    m_pheripherials = nullptr;
    m_pheriphCount = 0;
}

GUI::~GUI()
{
    if (m_pheripherials)
    {
        delete[] m_pheripherials;
    }
}

bool GUI::windowClosed()
{
    return end;
}