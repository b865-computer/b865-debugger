#include "Clock.h"
#include <thread>
#include <chrono>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#endif
#include <math.h>

template<typename tVal>
tVal map_value(std::pair<tVal,tVal> a, std::pair<tVal, tVal> b, tVal inVal)
{
  tVal inValNorm = inVal - a.first;
  tVal aUpperNorm = a.second - a.first;
  tVal normPosition = inValNorm / aUpperNorm;

  tVal bUpperNorm = b.second - b.first;
  tVal bValNorm = normPosition * bUpperNorm;
  tVal outVal = b.first + bValNorm;

  return outVal;
}

void pinThreadToCore(std::thread::native_handle_type handle, int core_id)
{
#ifdef _WIN32

    DWORD_PTR mask = (1ULL << core_id);
    if (!SetThreadAffinityMask((HANDLE)handle, mask))
    {
        fprintf(stderr, "Failed to set thread affinity. Error: %lu\n", GetLastError());
    }
#else
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    if (pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset) != 0)
    {
        fprintf(stderr, "Failed to set thread affinity.\n");
    }
#endif
}

void setThreadPriority(std::thread::native_handle_type handle, bool high_priority)
{
#ifdef _WIN32
    int priority = high_priority ? THREAD_PRIORITY_HIGHEST : THREAD_PRIORITY_NORMAL;
    if (!SetThreadPriority((HANDLE)handle, priority))
    {
        fprintf(stderr, "Failed to set thread priority. Error: %lu\n", GetLastError());
    }
#else
    sched_param sch_params;
    sch_params.sched_priority = high_priority ? sched_get_priority_max(SCHED_FIFO) : 0;

    if (pthread_setschedparam(handle, high_priority ? SCHED_FIFO : SCHED_OTHER, &sch_params) != 0)
    {
        fprintf(stderr, "Failed to set thread priority.\n");
    }
#endif
}



// Dummy m_cycle_func for demonstration purposes
void m_cycle_func()
{
    // Replace with actual functionality
}

// Constructor with default HZ
FQ::FQ(uint64_t _HZ)
{
    set(_HZ);
}

// Copy constructor
FQ::FQ(const FQ& other)
{
    set(other.HZ);
}

// Method to set frequency
void FQ::set(uint64_t _HZ)
{
    if (_HZ == 0)
    {
        _HZ = 1000000; // Default to 1 MHz
    }
    HZ = _HZ;
    ns = 1000000000 / HZ;
    sleep = (ns < sleep_threshold) ? 0 : default_sleep;
}

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
        m_fq.set(m_targetFq.HZ);
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
    bool running = m_isRunning;
    setStatus(false);
    double ratio = m_fq.HZ / m_targetFq.HZ;
    if(ratio < 1.0)
    {
        ratio = 1.0;
    }
    m_fq.set(_HZ * ratio);
    m_targetFq.set(_HZ);

    // thresholds for adjustments
    if(m_targetFq.HZ < 1000)
    {
        upper_threshold = m_targetFq.HZ + 1;
        lower_threshold = m_targetFq.HZ - 1;
    }
    else
    {
        upper_threshold = m_targetFq.HZ + 50;
        lower_threshold = m_targetFq.HZ - 50;
    }

    std::pair<double,double> freq(log10(max_frequency), 1.0f);
    std::pair<double,double> interval(min_reset_interval.count(), max_reset_interval.count());
    reset_interval = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds((uint64_t)map_value(freq, interval, log10((double)m_targetFq.HZ))));
    
    setStatus(running);
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
    // Pin the thread to a core and set its priority
#ifdef _WIN32
    pinThreadToCore((std::thread::native_handle_type)GetCurrentThread(), 0); // Pin to core 0
    setThreadPriority((std::thread::native_handle_type)GetCurrentThread(), true); // Set high priority
#else
    pinThreadToCore(clockThread.native_handle(), 0); // Pin to core 0
    setThreadPriority(clockThread.native_handle(), true); // Set high priority
#endif

    bool sleep = (m_fq.sleep > 25);
    counter = 0;

    m_start = std::chrono::high_resolution_clock::now();
    auto last_time = m_start;
    auto cycle_duration = std::chrono::nanoseconds(m_fq.ns);

    if(m_targetFq.HZ < 1000)
    {
        upper_threshold = m_targetFq.HZ + 1;
        lower_threshold = m_targetFq.HZ - 1;
    }
    else
    {
        upper_threshold = m_targetFq.HZ + 50;
        lower_threshold = m_targetFq.HZ - 50;
    }

    reset_interval = std::chrono::nanoseconds((unsigned long long)1e9);
    
    while (!end)
    {
        m_newStatus = m_isRunning;
        if (!m_isRunning)
        {
            m_start = std::chrono::high_resolution_clock::now();
            last_time = m_start;
            counter = 0;
            cycle_duration = std::chrono::nanoseconds(m_fq.ns);

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (!m_tick)
            {
                continue;
            }
        }

        m_now = std::chrono::high_resolution_clock::now();
        m_elapsed = m_now - last_time;

        if (m_elapsed >= cycle_duration || m_tick)
        {
            m_tick = false;
            last_time = m_now;
            m_cycle_func();
            counter++;

            if ((m_now - m_start) >= reset_interval) {
                counter = 1;  // Reset the cycle counter
                m_start = m_now;  // Reset start time
                continue;
            }

            // Adjust frequency if needed
            double current_frequency = static_cast<double>(counter) / 
                                    (std::chrono::duration<double>(m_now - m_start).count());

            // Only make adjustments if we're outside the dead zone
            if (current_frequency > upper_threshold)
            {
                cycle_duration += std::chrono::nanoseconds((uint64_t)((current_frequency - upper_threshold) / 20));
            }
            else if (current_frequency < lower_threshold)
            {
                cycle_duration -= std::chrono::nanoseconds((uint64_t)((lower_threshold - current_frequency) / 20));
            }
        }

        if (sleep)
        {
            std::this_thread::sleep_for(std::chrono::nanoseconds(m_fq.sleep));
        }
    }
}
