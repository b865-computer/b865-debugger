#pragma once
#ifndef _CLOCK_H_
#define _CLOCK_H_

#include "Common.h"
#include "atomic"

class FQ
{
public:
    // Constructor with default HZ
    FQ(uint64_t _HZ = 1000000);

    // Copy constructor
    FQ(const FQ& other);

    // Method to set frequency
    void set(uint64_t _HZ);

    // Member variables
    uint64_t HZ = 1000000;                // Frequency in Hertz
    uint64_t ns = 1000;                   // Nanoseconds per cycle
    uint64_t sleep = 10;                  // Sleep duration in nanoseconds

private:
    static constexpr uint64_t default_sleep = 50; // Default sleep duration
    static constexpr uint64_t sleep_threshold = 250; // Threshold for disabling sleep
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
    void (*m_cycle_func)(void);

private:
    void clockThreadFunc();

private:
    uint64_t counter = 0; // not it's not gonna overflow for 29.2 years. (at 20 MHz)
    std::thread clockThread;
    bool m_newStatus = false;
    std::atomic<bool> end;
    std::atomic<bool> m_isRunning;
    std::atomic<bool> m_tick;
    FQ m_fq;
    FQ m_targetFq;
    std::chrono::_V2::system_clock::time_point m_start;
    std::chrono::_V2::system_clock::time_point m_now;
    std::chrono::nanoseconds m_elapsed;
    double upper_threshold;
    double lower_threshold;
    std::chrono::nanoseconds reset_interval;
    const double max_frequency = 10e6;  // 10 MHz
    const std::chrono::seconds max_reset_interval = std::chrono::seconds(10);  // Maximum reset interval (10 seconds for 1 Hz)
    const std::chrono::seconds min_reset_interval = std::chrono::seconds(1);
};

#endif