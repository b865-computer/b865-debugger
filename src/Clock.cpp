#include "Clock.h"
#include <chrono>

Clock::Clock(void (*_cycle_func)(void))
    : m_fq(1), m_cycle_func(_cycle_func)
{
}

void Clock::start()
{
    clockThread = std::thread(clockThreadFunc, this, &end);
}

void Clock::stop()
{
    m_elapsed = std::chrono::high_resolution_clock::now() - m_start;
    end = true;
    if (clockThread.joinable())
    {
        clockThread.join();
    }
}

void Clock::setHZ(uint64_t _HZ)
{
    m_fq.set(_HZ);
}

uint64_t Clock::getHZ()
{
    return m_fq.HZ;
}

/**
 * @return the time elapsed since the clock started.
 * the time is based on the cycles executed by the clock, not
 * the real elapsed time.
 */
unsigned long long Clock::getRunTimeCycles_ns()
{
    return counter * m_fq.ns;
}

std::chrono::nanoseconds Clock::getRunTime_ns()
{
    return m_elapsed;
}

unsigned long long Clock::getCycles()
{
    return counter;
}

void Clock::clockThreadFunc(bool *_end)
{
    bool sleep = (m_fq.sleep > 100);

    m_start = std::chrono::high_resolution_clock::now();
    m_now = m_start;
    m_elapsed = std::chrono::high_resolution_clock::now() - m_start;

    while (!(*_end))
    {
        m_now = std::chrono::high_resolution_clock::now();
        m_elapsed = m_now - (m_start + std::chrono::nanoseconds(counter * m_fq.ns));
        uint64_t nanoseconds = m_elapsed.count();
        if (nanoseconds > m_fq.ns)
        {
            m_cycle_func();
            counter++;
        }
        if (sleep)
        {
            std::this_thread::sleep_for(std::chrono::nanoseconds(m_fq.sleep));
        }
    }
}
