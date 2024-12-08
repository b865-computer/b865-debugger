#pragma once
#ifndef _CLOCK_H_
#define _CLOCK_H_

#include "Common.h"
#include <thread>

class FQ
{
public:
    inline FQ(uint64_t _HZ)
    {
        if(_HZ == 0)
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
        sleep = ns / 5;
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
    void start();
    void stop();
    void setHZ(uint64_t _HZ);
    uint64_t getHZ();
    unsigned long long getRunTimeCycles_ns();
    std::chrono::nanoseconds getRunTime_ns();
    unsigned long long getCycles();

private:
    void clockThreadFunc(bool *end);

private:
    unsigned long long counter = 0; // not it's not gonna overflow for 584 years. (at 1 GHz)
    std::thread clockThread;
    bool end;
    FQ m_fq;
    void (*m_cycle_func)(void);
    std::chrono::_V2::system_clock::time_point m_start;
    std::chrono::_V2::system_clock::time_point m_now;
    std::chrono::nanoseconds m_elapsed;
};

#endif