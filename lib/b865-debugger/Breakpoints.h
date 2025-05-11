#pragma once
#ifndef _B865_DB_BREAKPOINT_H_
#define _B865_DB_BREAKPOINT_H_

#include <cstdint>
#include <string>
#include <vector>
#include "Parser/DebuggerData.h"

class Breakpoint
{
public:
    std::string file;
    size_t line;
    size_t block;
    size_t level;
    uint16_t addr;
    bool asmFile = false;
    size_t id = 0;

    int setPos(std::string& file, size_t& line, DebuggerData* data);
    uint16_t getAddr();
};

class BreakpointData
{
public:
    size_t id;
    size_t addr;
    std::string func;
    std::string file;
    std::string fullname;
    size_t line;
    std::string original_loc;
};

class BreakpointList
{
public:
    static bool print;
    static std::string execPath;
    std::vector<Breakpoint> breakpoints;
    std::vector<uint16_t> addresses;
    size_t id = 0;
    BreakpointData addBreakpoint(std::string& file, size_t& line, DebuggerData* data);
    BreakpointData addBreakpoint(const std::vector<std::string> &args, DebuggerData* data);
    void delBreakpoint(size_t id);
    void delBreakpoint(const std::vector<std::string> &args);
    void updateAddresses();
};

#endif