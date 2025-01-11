#include "Clock.h"

Clock::Clock(void (*_cycle_func)(void))
    : m_fq(1), m_targetFq(1), m_cycle_func(_cycle_func)
{
}

void Clock::init()
{
    end = false;
    clockThread = std::thread(clockThreadFunc, this);
}

void Clock::terminate()
{
    m_elapsed = std::chrono::high_resolution_clock::now() - m_start;
    end = true;
    if (clockThread.joinable())
    {
        clockThread.join();
    }
}

void Clock::setStatus(bool running)
{
    if (m_isRunning != running && running)
    {
        m_start = std::chrono::high_resolution_clock::now();
    }
    m_isRunning = running;
    if (!m_isRunning)
    {
        counter = 0;
    }
    while (m_isRunning != m_newStatus)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

bool Clock::getStatus()
{
    return m_isRunning;
}

void Clock::singleCycle()
{
    m_tick = true;
}

void Clock::setHZ(uint64_t _HZ)
{
    m_fq.set(_HZ);
    m_targetFq.set(_HZ);
}

uint64_t Clock::getHZ()
{
    return m_targetFq.HZ;
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
    return m_now - m_start;
}

unsigned long long Clock::getCycles()
{
    return counter;
}

void Clock::clockThreadFunc()
{
    bool sleep = (m_fq.sleep > 25);
    counter = 0;

    m_start = std::chrono::high_resolution_clock::now();
    m_now = m_start;
    auto last_start = m_start;
    m_elapsed = std::chrono::high_resolution_clock::now() - m_start;

    while (!end)
    {
        m_newStatus = m_isRunning;
        if (!m_isRunning)
        {
            m_start = std::chrono::high_resolution_clock::now();
            last_start = m_start;
            counter = 0;
            std::this_thread::sleep_for(std::chrono::nanoseconds(1000));
            if (!m_tick)
            {
                continue;
            }
        }
        m_now = std::chrono::high_resolution_clock::now();
        m_elapsed = m_now - last_start;
        if (m_elapsed.count() > m_fq.ns || m_tick)
        {
            last_start = m_now;
            m_tick = false;
            m_cycle_func();
            counter++;
            if(((double)getCycles() / ((double)(m_now - m_start).count() / 1e9)) > m_targetFq.HZ)
            {
                m_fq.set(m_fq.HZ - 1);
            }
            if(((double)getCycles() / ((double)(m_now - m_start).count() / 1e9)) < m_targetFq.HZ)
            {
                m_fq.set(m_fq.HZ + 1);
            }
        }
        if (sleep)
        {
            std::this_thread::sleep_for(std::chrono::nanoseconds(m_fq.sleep));
        }
    }
}
