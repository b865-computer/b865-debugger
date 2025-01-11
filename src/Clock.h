#pragma once
#ifndef _CLOCK_H_
#define _CLOCK_H_

#include "Common.h"

class FQ
{
public:
    inline FQ(uint64_t _HZ)
    {
        if (_HZ == 0)
        {
            _HZ = 1000000;
        }
        set(_HZ);
    }
    inline FQ(FQ &_fq)
    {
        FQ(_fq.HZ);
    }
    inline void set(uint64_t _HZ)
    {
        HZ = _HZ;
        ns = 1000000000 / HZ;
        sleep = 50;
        if (ns < 250)
        {
            sleep = 0;
        }
        return;
    }
    uint64_t sleep = 10;
    uint64_t ns = 10;
    uint64_t HZ = 10;
};

class Clock
{
public:
    Clock(void (*_cycle_func)(void));
    void init();
    void terminate();
    void setStatus(bool);
    bool getStatus();
    void singleCycle();
    void setHZ(uint64_t _HZ);
    uint64_t getHZ();
    uint64_t getRunTimeCycles_ns();
    std::chrono::nanoseconds getRunTime_ns();
    uint64_t getCycles();

private:
    void clockThreadFunc();

private:
    uint64_t counter = 0; // not it's not gonna overflow for 584 years. (at 1 GHz)
    std::thread clockThread;
    bool m_tick;
    bool m_isRunning = false;
    bool m_newStatus = false;
    bool end;
    FQ m_fq;
    FQ m_targetFq;
    void (*m_cycle_func)(void);
    std::chrono::_V2::system_clock::time_point m_start;
    std::chrono::_V2::system_clock::time_point m_now;
    std::chrono::nanoseconds m_elapsed;
};

#endif